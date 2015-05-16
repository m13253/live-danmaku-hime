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
#include "../app.h"
#include "../config.h"
#include <cassert>
#include <cstdlib>
#include <map>
#include <windows.h>

namespace dmhm {

struct GDIPresenterPrivate {
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static std::map<HWND, GDIPresenter *> hWndMap; // tree_map is enogh, not necessarily need a unordered_map
    Application *app = nullptr;
    HINSTANCE hInstance = nullptr;
    HWND hWnd = nullptr;
    HDC window_dc = nullptr;
    HDC buffer_dc = nullptr;
    HBITMAP buffer_bmp = nullptr;
    void create_buffer(GDIPresenter *pub);
    void do_paint(GDIPresenter *pub, uint32_t *bitmap, int32_t width, int32_t height);
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
    p->hWnd = CreateWindowExW(
        WS_EX_LAYERED,
        reinterpret_cast<LPCWSTR>(wnd_class_atom),
        L"\u5f39\u5e55\u59ec",
        WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr,
        nullptr,
        p->hInstance,
        nullptr
    );
    if(!p->hWnd) {
        /* Failed to create window */
        report_error("\xe5\x88\x9b\xe5\xbb\xba\xe7\xaa\x97\xe5\x8f\xa3\xe5\xa4\xb1\xe8\xb4\xa5");
        abort();
    }
    p->hWndMap[p->hWnd] = this;

    p->create_buffer(this);
}

GDIPresenter::~GDIPresenter() {
    if(p->hWnd)
        DestroyWindow(p->hWnd);
    if(p->window_dc)
        ReleaseDC(p->hWnd, p->window_dc);
    if(p->buffer_dc)
        DeleteDC(p->buffer_dc);
    if(p->buffer_bmp)
        DeleteObject(p->buffer_bmp);
}

void GDIPresenter::report_error(const std::string error) {
    MessageBoxW(nullptr, utf8_to_wide(error, false).c_str(), nullptr, MB_ICONERROR);
}

void GDIPresenter::get_stage_rect(int32_t &top, int32_t &left, int32_t &right, int32_t &bottom) {
    HMONITOR monitor = MonitorFromWindow(p->hWnd, MONITOR_DEFAULTTOPRIMARY);
    if(!monitor) {
        /* Failed to retrieve screen size */
        report_error("\xe8\x8e\xb7\xe5\x8f\x96\xe5\xb1\x8f\xe5\xb9\x95\xe5\xb0\xba\xe5\xaf\xb8\xe5\xa4\xb1\xe8\xb4\xa5");
        abort();
    }
    MONITORINFO monitor_info;
    monitor_info.cbSize = sizeof monitor_info;
    if(!GetMonitorInfoW(monitor, &monitor_info)) {
        /* Failed to retrieve screen size */
        report_error("\xe8\x8e\xb7\xe5\x8f\x96\xe5\xb1\x8f\xe5\xb9\x95\xe5\xb0\xba\xe5\xaf\xb8\xe5\xa4\xb1\xe8\xb4\xa5");
        abort();
    }
    top = monitor_info.rcWork.top;
    left = monitor_info.rcWork.right - config::stage_width;
    right = monitor_info.rcWork.right;
    bottom = monitor_info.rcWork.bottom;
}

void GDIPresenterPrivate::create_buffer(GDIPresenter *pub) {
    if(!window_dc)
        window_dc = GetDC(hWnd);
    if(!buffer_dc)
        buffer_dc = CreateCompatibleDC(window_dc);

    int32_t top, left, right, bottom;
    pub->get_stage_rect(top, left, right, bottom);
    if(buffer_bmp)
        DeleteObject(buffer_bmp);
    buffer_bmp = CreateCompatibleBitmap(buffer_dc, right-left, bottom-top);
}

void GDIPresenterPrivate::do_paint(GDIPresenter *, uint32_t *bitmap, int32_t width, int32_t height) {
    BITMAPINFO bitmap_info;
    bitmap_info.bmiHeader.biSize = sizeof bitmap_info.bmiHeader;
    bitmap_info.bmiHeader.biWidth = width;
    bitmap_info.bmiHeader.biHeight = height;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;
    bitmap_info.bmiHeader.biSizeImage = 0;
    bitmap_info.bmiHeader.biXPelsPerMeter = 96;
    bitmap_info.bmiHeader.biYPelsPerMeter = 96;
    bitmap_info.bmiHeader.biClrUsed = 0;
    bitmap_info.bmiHeader.biClrImportant = 0;

    SetDIBits(buffer_dc, buffer_bmp, 0, height, bitmap, &bitmap_info, 0);
}

LRESULT CALLBACK GDIPresenterPrivate::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

std::map<HWND, GDIPresenter *> GDIPresenterPrivate::hWndMap;

}
