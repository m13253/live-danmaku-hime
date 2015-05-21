/*
  Copyright (c) 2015 StarBrilliant <m13253@hotmail.com>
  All rights reserved.

  Redistribution and use in source and binary forms are permitted
  provided that the above copyright notice and this paragraph are
  duplicated in all such forms and that any documentation,
  advertising materials, and other materials related to such
  distribution and use acknowledge that the software was developed by
  StarBrilliant.
  The name of StarBrilliant may not be used to endorse or promote
  products derived from this software without specific prior written
  permission.

  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "gdi.h"
#include "../utils.h"
#include "../app.h"
#include "../renderer/renderer.h"
#include "../config.h"
#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>
#include <windows.h>

namespace dmhm {

struct GDIPresenterPrivate {
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static std::map<HWND, GDIPresenter *> hWndMap; // tree_map is enogh, not necessarily need a unordered_map
    Application *app = nullptr;
    HINSTANCE hInstance = nullptr;
    HWND hWnd = nullptr;
    HDC screen_dc = nullptr;
    HDC window_dc = nullptr;
    HDC buffer_dc = nullptr;
    HBITMAP dib_handle = nullptr;
    uint32_t *dib_buffer = nullptr;
    int32_t top; int32_t left; int32_t right; int32_t bottom;
    void get_stage_rect(GDIPresenter *pub);
    void create_buffer(GDIPresenter *pub);
    void do_paint(GDIPresenter *pub, const uint32_t *bitmap, uint32_t width, uint32_t height, uint32_t stride);
};

GDIPresenter::GDIPresenter(Application *app) {
    p->app = app;
    p->hInstance = GetModuleHandleW(nullptr);

    //SetProcessDPIAwareness(Process_System_DPI_Aware);

    /* Register window class */
    WNDCLASSEXW wnd_class;
    wnd_class.cbSize = sizeof wnd_class;
    wnd_class.style = CS_HREDRAW | CS_VREDRAW;
    wnd_class.lpfnWndProc = p->WndProc;
    wnd_class.cbClsExtra = 0;
    wnd_class.cbWndExtra = 0;
    wnd_class.hInstance = p->hInstance;
    wnd_class.hIcon = reinterpret_cast<HICON>(LoadImageW(nullptr, reinterpret_cast<LPCWSTR>(IDI_APPLICATION), IMAGE_ICON, 0, 0, LR_SHARED));
    wnd_class.hCursor = reinterpret_cast<HCURSOR>(LoadImageW(nullptr, reinterpret_cast<LPCWSTR>(IDC_ARROW), IMAGE_CURSOR, 0, 0, LR_SHARED));
    wnd_class.hbrBackground = HBRUSH(GetStockObject(BLACK_BRUSH));
    wnd_class.lpszMenuName = nullptr;
    wnd_class.lpszClassName = L"com.starbrilliant.danmakuhime";
    wnd_class.hIconSm = nullptr;
    ATOM wnd_class_atom = RegisterClassExW(&wnd_class);
    if(wnd_class_atom == 0) {
        /* Failed to set window class */
        report_error("\xe8\xae\xbe\xe5\xae\x9a\xe7\xaa\x97\xe5\x8f\xa3\xe7\xb1\xbb\xe5\x9e\x8b\xe5\xa4\xb1\xe8\xb4\xa5");
        abort();
    }

    /* Create window */
    /* p->hWnd = */ CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
        reinterpret_cast<LPCWSTR>(wnd_class_atom),
        L"\u5f39\u5e55\u59ec",
        WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr,
        nullptr,
        p->hInstance,
        this
    );
    if(!p->hWnd) { /* hWnd should be filled in WndProc */
        /* Failed to create window */
        report_error("\xe5\x88\x9b\xe5\xbb\xba\xe7\xaa\x97\xe5\x8f\xa3\xe5\xa4\xb1\xe8\xb4\xa5");
        abort();
    }
}

GDIPresenter::~GDIPresenter() {
    p->hWndMap.erase(p->hWndMap.find(p->hWnd));
    if(p->hWnd) {
        DestroyWindow(p->hWnd);
        p->hWnd = nullptr;
    }
    if(p->screen_dc) {
        ReleaseDC(nullptr, p->screen_dc);
        p->screen_dc = nullptr;
    }
    if(p->window_dc) {
        ReleaseDC(p->hWnd, p->window_dc);
        p->window_dc = nullptr;
    }
    if(p->buffer_dc) {
        DeleteDC(p->buffer_dc);
        p->buffer_dc = nullptr;
    }
    if(p->dib_handle) {
        DeleteObject(p->dib_handle);
        p->dib_handle = nullptr;
        p->dib_buffer = nullptr;
    }
}

void GDIPresenter::report_error(const std::string error) {
    MessageBoxW(nullptr, utf8_to_wide(error).c_str(), nullptr, MB_ICONERROR);
}

void GDIPresenter::get_stage_size(uint32_t &width, uint32_t &height) {
    dmhm_assert(p->right >= p->left && p->bottom >= p->top);
    width = p->right - p->left;
    height = p->bottom - p->top;
}

void GDIPresenter::paint_frame() {
    uint32_t width, height;
    get_stage_size(width, height);

    Renderer *renderer = reinterpret_cast<Renderer *>(p->app->get_renderer());
    dmhm_assert(renderer);
    if(!renderer->paint_frame(width, height, [=](const uint32_t *bitmap, uint32_t stride) {
        p->do_paint(this, bitmap, width, height, stride);
    }))
        PostQuitMessage(0);
}

int GDIPresenter::run_loop() {
    MSG message;
    while(GetMessageW(&message, nullptr, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return 0;
}

void GDIPresenterPrivate::get_stage_rect(GDIPresenter *pub) {
    HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
    if(!monitor) {
        /* Failed to retrieve screen size */
        pub->report_error("\xe8\x8e\xb7\xe5\x8f\x96\xe5\xb1\x8f\xe5\xb9\x95\xe5\xb0\xba\xe5\xaf\xb8\xe5\xa4\xb1\xe8\xb4\xa5");
        abort();
    }
    MONITORINFO monitor_info;
    monitor_info.cbSize = sizeof monitor_info;
    if(!GetMonitorInfoW(monitor, &monitor_info)) {
        /* Failed to retrieve screen size */
        pub->report_error("\xe8\x8e\xb7\xe5\x8f\x96\xe5\xb1\x8f\xe5\xb9\x95\xe5\xb0\xba\xe5\xaf\xb8\xe5\xa4\xb1\xe8\xb4\xa5");
        abort();
    }
    top = monitor_info.rcWork.top;
    left = monitor_info.rcWork.right - config::stage_width;
    right = monitor_info.rcWork.right;
    bottom = monitor_info.rcWork.bottom;
}

void GDIPresenterPrivate::create_buffer(GDIPresenter *pub) {
    if(!screen_dc) {
        screen_dc = GetDC(nullptr);
        if(!screen_dc) {
            /* Failed to create buffer */
            pub->report_error("\xe5\x88\x9b\xe5\xbb\xba\xe7\xbc\x93\xe5\x86\xb2\xe5\x8c\xba\xe5\xa4\xb1\xe8\xb4\xa5");
            abort();
        }
    }
    if(!window_dc) {
        window_dc = GetDC(hWnd);
        if(!window_dc) {
            /* Failed to create buffer */
            pub->report_error("\xe5\x88\x9b\xe5\xbb\xba\xe7\xbc\x93\xe5\x86\xb2\xe5\x8c\xba\xe5\xa4\xb1\xe8\xb4\xa5");
            abort();
        }
    }
    if(!buffer_dc) {
        buffer_dc = CreateCompatibleDC(screen_dc);
        if(!buffer_dc) {
            /* Failed to create buffer */
            pub->report_error("\xe5\x88\x9b\xe5\xbb\xba\xe7\xbc\x93\xe5\x86\xb2\xe5\x8c\xba\xe5\xa4\xb1\xe8\xb4\xa5");
            abort();
        }
    }
    if(!dib_handle) {
        uint32_t width, height;
        pub->get_stage_size(width, height);

        dib_buffer = nullptr;

        BITMAPINFO bitmap_info;
        bitmap_info.bmiHeader.biSize = sizeof bitmap_info.bmiHeader;
        bitmap_info.bmiHeader.biWidth = width;
        bitmap_info.bmiHeader.biHeight = height;
        bitmap_info.bmiHeader.biPlanes = 1;
        bitmap_info.bmiHeader.biBitCount = 32;
        bitmap_info.bmiHeader.biCompression = BI_RGB;
        bitmap_info.bmiHeader.biSizeImage = 0;
        bitmap_info.bmiHeader.biXPelsPerMeter = 3780;
        bitmap_info.bmiHeader.biYPelsPerMeter = 3780;
        bitmap_info.bmiHeader.biClrUsed = 0;
        bitmap_info.bmiHeader.biClrImportant = 0;

        dib_handle = CreateDIBSection(screen_dc, &bitmap_info, DIB_RGB_COLORS, reinterpret_cast<void **>(&dib_buffer), nullptr, 0);
        if(!dib_handle) {
            /* Failed to create buffer */
            pub->report_error("\xe5\x88\x9b\xe5\xbb\xba\xe7\xbc\x93\xe5\x86\xb2\xe5\x8c\xba\xe5\xa4\xb1\xe8\xb4\xa5");
            abort();
        }
    }
    SelectObject(buffer_dc, dib_handle);
}

void GDIPresenterPrivate::do_paint(GDIPresenter *pub, const uint32_t *bitmap, uint32_t width, uint32_t height, uint32_t stride) {
    for(uint32_t i = 0; i < height; i++)
        for(uint32_t j = 0; j < width; j++) {
            /*
            uint8_t alpha = uint8_t(bitmap[i*stride + j] >> 24);
            uint32_t red = ((bitmap[i*stride + j] & 0xff0000) * alpha / 255) & 0xff0000;
            uint32_t green = ((bitmap[i*stride + j] & 0xff00) * alpha / 255) & 0xff00;
            uint32_t blue = ((bitmap[i*stride + j] & 0xff) * alpha / 255) & 0xff;
            dib_buffer[(height-i-1)*width + j] = (uint32_t(alpha) << 24) | red | green | blue;
            */
            dib_buffer[(height-i-1)*width + j] = bitmap[i*stride + j];
        }

    POINT window_pos;
    window_pos.x = left;
    window_pos.y = top;
    SIZE window_size;
    window_size.cx = right-left;
    window_size.cy = bottom-top;
    POINT buffer_pos;
    buffer_pos.x = 0;
    buffer_pos.y = 0;
    BLENDFUNCTION blend_function;
    blend_function.BlendOp = AC_SRC_OVER;
    blend_function.BlendFlags = 0;
    blend_function.SourceConstantAlpha = 255; // Set the SourceConstantAlpha value to 255 (opaque) when you only want to use per-pixel alpha values.
    blend_function.AlphaFormat = AC_SRC_ALPHA;
    if(!UpdateLayeredWindow(hWnd, window_dc, &window_pos, &window_size, buffer_dc, &buffer_pos, 0, &blend_function, ULW_ALPHA)) {
        /* Desktop compositor failed to set window transparency */
        pub->report_error("\xe6\xa1\x8c\xe9\x9d\xa2\xe6\xb7\xb7\xe6\x88\x90\xe5\x99\xa8\xe6\x97\xa0\xe6\xb3\x95\xe8\xae\xbe\xe7\xbd\xae\xe9\x80\x8f\xe6\x98\x8e\xe7\xaa\x97\xe5\x8f\xa3");
        abort();
    }
}

LRESULT CALLBACK GDIPresenterPrivate::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if(uMsg == WM_NCCREATE) {
        GDIPresenter *pub = reinterpret_cast<GDIPresenter *>(reinterpret_cast<CREATESTRUCTW *>(lParam)->lpCreateParams);
        hWndMap[hWnd] = pub;
        pub->p->hWnd = hWnd;
    } else {
        GDIPresenter *pub = hWndMap[hWnd];
        switch(uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_CREATE:
            pub->p->get_stage_rect(pub);
            pub->p->create_buffer(pub);
            ShowWindow(hWnd, SW_SHOW);
            if(!SetTimer(hWnd, 0, 16, nullptr)) {
                pub->report_error("\xe5\x90\xaf\xe5\x8a\xa8\xe5\x8a\xa8\xe7\x94\xbb\xe5\xae\x9a\xe6\x97\xb6\xe5\x99\xa8\xe5\xa4\xb1\xe8\xb4\xa5");
                abort();
            }
            break;
        case WM_TIMER:
            pub->paint_frame();
            break;
        }
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

std::map<HWND, GDIPresenter *> GDIPresenterPrivate::hWndMap;

}
