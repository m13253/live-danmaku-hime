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
#include <cairo/cairo-ft.h>
#include "freetype_includer.h"

extern "C" void _cairo_mutex_initialize();

namespace dmhm {

struct DanmakuAnimator;

struct CairoRendererPrivate {
    Application *app = nullptr;

    /* Stage size */
    uint32_t width = 0;
    uint32_t height = 0;

    FT_Library freetype = nullptr;
    FT_Face ft_font_face = nullptr;
    cairo_font_face_t *cairo_font_face = nullptr;

    cairo_surface_t *cairo_surface = nullptr;
    cairo_t *cairo_instance = nullptr;

    bool is_eof = false;
    std::list<DanmakuAnimator> danmaku_list;

    void create_cairo(cairo_surface_t *&cairo_surface, cairo_t *&cairo);
    static void release_cairo(cairo_surface_t *&cairo_surface, cairo_t *&cairo);
    void fetch_danmaku(std::chrono::steady_clock::time_point now);
    void animate_text(std::chrono::steady_clock::time_point now);
    void paint_text();
};

CairoRenderer::CairoRenderer(Application *app) {
    p->app = app;

    /* Initialize fonts */
    FT_Error ft_error;
    ft_error = FT_Init_FreeType(&p->freetype);
    assert(ft_error == 0);
    ft_error = FT_New_Face(p->freetype, config::font_file, config::font_file_index, &p->ft_font_face);
    assert(ft_error != FT_Err_Cannot_Open_Resource);
    assert(ft_error != FT_Err_Unknown_File_Format);
    assert(ft_error == 0);
    ft_error = FT_Set_Char_Size(p->ft_font_face, 0, FT_F26Dot6(config::font_size*64), 72, 72);
    assert(ft_error == 0);

    /* There is a bug in _cairo_ft_unscaled_font_map_lock,
       causing mutex not being initialized correctly.
       I will hack it dirtly by calling a private API. */
    _cairo_mutex_initialize();

    p->cairo_font_face = cairo_ft_font_face_create_for_ft_face(p->ft_font_face, 0);
}

CairoRenderer::~CairoRenderer() {
    p->release_cairo(p->cairo_surface, p->cairo_instance);

    FT_Error ft_error;
    if(p->cairo_font_face) {
        cairo_font_face_destroy(p->cairo_font_face);
        p->cairo_font_face = nullptr;
    }
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

    cairo_set_font_face(p->cairo_instance, p->cairo_font_face);
    cairo_set_font_size(p->cairo_instance, config::font_size);

    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    p->fetch_danmaku(now);
    p->animate_text(now);
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
    double x;
    double y;
    double height;
    double alpha = 1;
    bool moving = false;
    std::chrono::steady_clock::time_point starttime;
    std::chrono::steady_clock::time_point endtime;
    double starty;
    double endy;
};

void CairoRendererPrivate::fetch_danmaku(std::chrono::steady_clock::time_point now) {
    Fetcher *fetcher = reinterpret_cast<Fetcher *>(app->get_fetcher());
    assert(fetcher);

    is_eof = fetcher->is_eof();
    fetcher->pop_messages([&](DanmakuEntry &entry) {
        DanmakuAnimator animator(entry);
        animator.y = height-config::shadow_radius;
        cairo_text_extents_t text_extents;
        cairo_text_extents(cairo_instance, animator.entry.message.c_str(), &text_extents);
        animator.height = text_extents.height+config::extra_line_height;
        for(DanmakuAnimator &i : danmaku_list) {
            i.starttime = now;
            i.endtime = now + std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(config::danmaku_attack));
            i.starty = i.y;
            if(i.moving)
                i.endy -= animator.height;
            else {
                i.endy = i.starty-animator.height;
                i.moving = true;
            }
        }
        danmaku_list.push_front(std::move(animator));
    });
}

void CairoRendererPrivate::animate_text(std::chrono::steady_clock::time_point now) {
    danmaku_list.remove_if([&](const DanmakuAnimator &x) -> bool {
        double timespan = double((now-x.entry.timestamp).count())*std::chrono::steady_clock::period::num/std::chrono::steady_clock::period::den;
        return timespan >= config::danmaku_lifetime;
    });
    for(DanmakuAnimator &i : danmaku_list) {
        double timespan = double((now-i.entry.timestamp).count())*std::chrono::steady_clock::period::num/std::chrono::steady_clock::period::den;
        if(timespan < config::danmaku_attack) {
            double progress = timespan/config::danmaku_attack;
            i.x = (width-config::shadow_radius)*(1-progress*(2-progress))+config::shadow_radius;
            i.alpha = progress;
        } else if(timespan < config::danmaku_lifetime-config::danmaku_decay) {
            i.x = config::shadow_radius;
            i.alpha = 1;
        } else {
            i.x = config::shadow_radius;
            i.alpha = (config::danmaku_lifetime-timespan)/config::danmaku_decay;
        }
        if(i.moving)
            if(now >= i.endtime) {
                i.moving = false;
                i.y = i.endy;
            } else
                i.y = i.starty+(i.endy-i.starty)*(now-i.starttime).count()/(i.endtime-i.starttime).count();
        else;
    }
}

void CairoRendererPrivate::paint_text() {
    for(const DanmakuAnimator &i : danmaku_list) {
        cairo_move_to(cairo_instance, i.x, i.y);
        cairo_set_source_rgba(cairo_instance, 1, 1, 1, i.alpha);
        cairo_show_text(cairo_instance, i.entry.message.c_str());
        cairo_stroke(cairo_instance);
    }
}

}
