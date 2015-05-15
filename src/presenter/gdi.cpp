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
#include <cstdlib>
#include <windows.h>

namespace dmhm {

struct GDIPresenterPrivate {
    Application *app;
    HINSTANCE hInstance;
    HWND hWnd;
};

GDIPresenter::GDIPresenter(Application *app) {
    p->app = app;
    p->hInstance = GetModuleHandleW(nullptr);
    /* Register window class */
    WNDCLASSEXW wnd_class = {
        .cbSize = sizeof (WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = nullptr,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = p->hInstance,
        .hIcon = LoadIcon(nullptr, IDI_APPLICATION),
        .hCursor = LoadCursor(nullptr, IDC_ARROW),
        .hbrBackground = HBRUSH(GetStockObject(BLACK_BRUSH)),
        .lpszMenuName = nullptr,
        .lpszClassName = L"com.starbrilliant.danmakuhime",
        .hIconSm = nullptr
    };
    ATOM wnd_class_atom = RegisterClassExW(&wnd_class);
    if(wnd_class_atom == 0) {
        /* Failed to set window class */
        report_error("\xe8\xae\xbe\xe5\xae\x9a\xe7\xaa\x97\xe5\x8f\xa3\xe7\xb1\xbb\xe5\x9e\x8b\xe5\xa4\xb1\xe8\xb4\xa5");
        abort();
    }
    /* Create window */
    p->hWnd = CreateWindowExW(
        WS_EX_LAYERED,
        reinterpret_cast<const wchar_t *>(wnd_class_atom),
        L"\u5f39\u5e55\u59ec",
        0,
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
}

GDIPresenter::~GDIPresenter() {
}

void GDIPresenter::report_error(const std::string error) {
    MessageBoxW(nullptr, utf8_to_wide(error, false).c_str(), nullptr, MB_ICONERROR);
}

}
