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

#include "cairo_render.h"
#include "../utils.h"
#include "../app.h"
#include "../config.h"
#include "../fetcher/fetcher.h"
#include <cassert>
#include <chrono>
#include <functional>
#include <list>
#include <cairo/cairo.h>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace dmhm {

struct DanmakuAnimator;

struct CairoRendererPrivate {
    Application *app = nullptr;

    /* Stage size */
    uint32_t width = 0;
    uint32_t height = 0;

    FT_Library freetype = nullptr;
    FT_Face ft_font_face = nullptr;

    cairo_surface_t *cairo_surface = nullptr;
    cairo_t *cairo_instance = nullptr;

    bool is_eof = false;
    std::list<DanmakuAnimator> danmaku_list;

    void create_cairo(cairo_surface_t *&cairo_surface, cairo_t *&cairo);
    static void release_cairo(cairo_surface_t *&cairo_surface, cairo_t *&cairo);
    void fetch_danmaku();
    void animate_text();
    void paint_text();
};

CairoRenderer::CairoRenderer(Application *app) {
    p->app = app;

    /* Initialize FreeType */
    FT_Error ft_error;
    ft_error = FT_Init_FreeType(&p->freetype);
    assert(ft_error == 0);
    ft_error = FT_New_Face(p->freetype, dmhm::config::font_file, dmhm::config::font_file_index, &p->ft_font_face);
    assert(ft_error != FT_Err_Unknown_File_Format);
    assert(ft_error == 0);
    ft_error = FT_Set_Char_Size(p->ft_font_face, 0, FT_F26Dot6(dmhm::config::font_size*64), 72, 72);
    assert(ft_error == 0);
}

CairoRenderer::~CairoRenderer() {
    p->release_cairo(p->cairo_surface, p->cairo_instance);

    FT_Error ft_error;
    if(p->ft_font_face) {
        ft_error = FT_Done_Face(p->ft_font_face);
        assert(ft_error == 0);
        p->ft_font_face = nullptr;
    }
    if(p->freetype) {
        ft_error = FT_Done_FreeType(p->freetype);
        assert(ft_error == 0);
        p->freetype = nullptr;
    }
}

bool CairoRenderer::paint_frame(uint32_t width, uint32_t height, std::function<void (const uint32_t *bitmap, uint32_t stride)> callback) {
    if(width != p->width || height != p->height) {
        p->width = width;
        p->height = height;
        p->release_cairo(p->cairo_surface, p->cairo_instance);
    }
    if(!p->cairo_instance)
        p->create_cairo(p->cairo_surface, p->cairo_instance);

    cairo_save(p->cairo_instance);
    cairo_set_operator(p->cairo_instance, CAIRO_OPERATOR_CLEAR);
    cairo_paint(p->cairo_instance);
    cairo_restore(p->cairo_instance);

    p->fetch_danmaku();
    p->animate_text();
    p->paint_text();

    callback(reinterpret_cast<uint32_t *>(cairo_image_surface_get_data(p->cairo_surface)), uint32_t(cairo_image_surface_get_stride(p->cairo_surface)/sizeof (uint32_t)));

    return !p->is_eof || !p->danmaku_list.empty();
}

void CairoRendererPrivate::create_cairo(cairo_surface_t *&cairo_surface, cairo_t *&cairo) {
    cairo_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo = cairo_create(cairo_surface);
}

void CairoRendererPrivate::release_cairo(cairo_surface_t *&cairo_surface, cairo_t *&cairo) {
    if(cairo) {
        cairo_destroy(cairo);
        cairo = nullptr;
    }
    if(cairo_surface) {
        cairo_surface_destroy(cairo_surface);
        cairo_surface = nullptr;
    }
}

struct DanmakuAnimator {
    DanmakuAnimator(const DanmakuEntry &entry) :
        entry(entry) {
    }
    DanmakuEntry entry;
    uint32_t x;
    uint32_t y;
    uint32_t height;
    double alpha = 1;
    bool moving = false;
    uint32_t desty;
    std::chrono::steady_clock::time_point desttime;
};

void CairoRendererPrivate::fetch_danmaku() {
    Fetcher *fetcher = reinterpret_cast<Fetcher *>(app->get_fetcher());
    assert(fetcher);

    is_eof = fetcher->is_eof();
    fetcher->pop_messages([&](DanmakuEntry &entry) {
        DanmakuAnimator animator(entry);
        animator.y = height;
        danmaku_list.push_front(std::move(animator));
    });
}

void CairoRendererPrivate::animate_text() {
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    danmaku_list.remove_if([=](const DanmakuAnimator &x) -> bool {
        return (x.entry.timestamp-now).count() >= dmhm::config::danmaku_lifetime;
    });
    for(DanmakuAnimator &i : danmaku_list) {
        if((i.entry.timestamp-now).count() >= dmhm::config::danmaku_attack) {
            i.x = dmhm::config::font_size;
            i.alpha = 1;
        }
        // TODO
    }
}

void CairoRendererPrivate::paint_text() {
    // TODO
}

}
