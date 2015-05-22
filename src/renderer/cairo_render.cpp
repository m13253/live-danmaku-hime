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
#include <cstdlib>
#include <chrono>
#include <functional>
#include <list>
#include <vector>
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

    cairo_surface_t *cairo_blend_surface = nullptr;
    cairo_t *cairo_blend_layer = nullptr;
    cairo_surface_t *cairo_text_surface = nullptr;
    cairo_t *cairo_text_layer = nullptr;

    bool is_eof = false;
    std::list<DanmakuAnimator> danmaku_list;

    void create_cairo(cairo_surface_t *&cairo_surface, cairo_t *&cairo);
    static void release_cairo(cairo_surface_t *&cairo_surface, cairo_t *&cairo);
    void fetch_danmaku(std::chrono::steady_clock::time_point now);
    void animate_text(std::chrono::steady_clock::time_point now);
    void paint_text();
    void blend_layers();

    std::vector<double> blur_kernel;
    void generate_blur_kernel(uint32_t radius);
    double get_blur_kernel(uint32_t radius, int32_t x, int32_t y);
};

CairoRenderer::CairoRenderer(Application *app) {
    p->app = app;

    /* Initialize fonts */
    FT_Error ft_error;
    ft_error = FT_Init_FreeType(&p->freetype);
    dmhm_assert(ft_error == 0);
    ft_error = FT_New_Face(p->freetype, config::font_file, config::font_file_index, &p->ft_font_face);
    dmhm_assert(ft_error != FT_Err_Cannot_Open_Resource);
    dmhm_assert(ft_error != FT_Err_Unknown_File_Format);
    dmhm_assert(ft_error == 0);
    ft_error = FT_Set_Char_Size(p->ft_font_face, 0, FT_F26Dot6(config::font_size*64), 72, 72);
    dmhm_assert(ft_error == 0);

    /* There is a bug in _cairo_ft_unscaled_font_map_lock,
       causing mutex not being initialized correctly.
       I will hack it dirtly by calling a private API. */
    _cairo_mutex_initialize();
    p->cairo_font_face = cairo_ft_font_face_create_for_ft_face(p->ft_font_face, 0);

    p->generate_blur_kernel(config::shadow_radius);
}

CairoRenderer::~CairoRenderer() {
    p->release_cairo(p->cairo_text_surface, p->cairo_text_layer);
    p->release_cairo(p->cairo_blend_surface, p->cairo_blend_layer);

    FT_Error ft_error;
    if(p->cairo_font_face) {
        cairo_font_face_destroy(p->cairo_font_face);
        p->cairo_font_face = nullptr;
    }
    if(p->ft_font_face) {
        ft_error = FT_Done_Face(p->ft_font_face);
        dmhm_assert(ft_error == 0);
        p->ft_font_face = nullptr;
    }
    if(p->freetype) {
        ft_error = FT_Done_FreeType(p->freetype);
        dmhm_assert(ft_error == 0);
        p->freetype = nullptr;
    }
}

bool CairoRenderer::paint_frame(uint32_t width, uint32_t height, std::function<void (const uint32_t *bitmap, uint32_t stride)> callback) {
    if(width != p->width || height != p->height) {
        p->width = width;
        p->height = height;
        p->release_cairo(p->cairo_text_surface, p->cairo_text_layer);
        p->release_cairo(p->cairo_blend_surface, p->cairo_blend_layer);
    }
    if(!p->cairo_blend_layer)
        p->create_cairo(p->cairo_blend_surface, p->cairo_blend_layer);
    if(!p->cairo_text_layer) {
        p->create_cairo(p->cairo_text_surface, p->cairo_text_layer);
        cairo_set_font_face(p->cairo_text_layer, p->cairo_font_face);
        cairo_set_font_size(p->cairo_text_layer, config::font_size);
        cairo_font_options_t *font_options = cairo_font_options_create();
        cairo_get_font_options(p->cairo_text_layer, font_options);
        cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_GRAY);
        cairo_font_options_set_hint_style(font_options, CAIRO_HINT_STYLE_NONE);
        cairo_font_options_set_hint_metrics(font_options, CAIRO_HINT_METRICS_OFF);
        cairo_set_font_options(p->cairo_text_layer, font_options);
        cairo_font_options_destroy(font_options);
    }

    cairo_set_operator(p->cairo_text_layer, CAIRO_OPERATOR_CLEAR);
    cairo_paint(p->cairo_text_layer);
    cairo_set_operator(p->cairo_text_layer, CAIRO_OPERATOR_OVER);

    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    p->fetch_danmaku(now);
    p->animate_text(now);
    p->paint_text();

    cairo_set_operator(p->cairo_blend_layer, CAIRO_OPERATOR_CLEAR);
    cairo_paint(p->cairo_blend_layer);
    cairo_set_operator(p->cairo_blend_layer, CAIRO_OPERATOR_OVER);
    p->blend_layers();

    cairo_surface_flush(p->cairo_blend_surface);
    callback(reinterpret_cast<uint32_t *>(cairo_image_surface_get_data(p->cairo_blend_surface)), uint32_t(cairo_image_surface_get_stride(p->cairo_blend_surface)/sizeof (uint32_t)));

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
    dmhm_assert(fetcher);

    is_eof = fetcher->is_eof();
    fetcher->pop_messages([&](DanmakuEntry &entry) {
        DanmakuAnimator animator(entry);
        animator.y = height-config::shadow_radius;
        cairo_text_extents_t text_extents;
        cairo_text_extents(cairo_text_layer, animator.entry.message.c_str(), &text_extents);
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
        cairo_move_to(cairo_text_layer, i.x, i.y);
        cairo_set_source_rgba(cairo_text_layer, 1, 1, 1, i.alpha);
        cairo_show_text(cairo_text_layer, i.entry.message.c_str());
        cairo_stroke(cairo_text_layer);
    }
}

void CairoRendererPrivate::blend_layers() {
    cairo_surface_flush(cairo_text_surface);
    cairo_surface_flush(cairo_blend_surface);
    const uint32_t *text_bitmap = reinterpret_cast<uint32_t *>(cairo_image_surface_get_data(cairo_text_surface));
    uint32_t text_stride = uint32_t(cairo_image_surface_get_stride(cairo_text_surface)/sizeof (uint32_t));
    cairo_surface_flush(cairo_blend_surface);
    uint32_t *blend_bitmap = reinterpret_cast<uint32_t *>(cairo_image_surface_get_data(cairo_blend_surface));
    uint32_t blend_stride = uint32_t(cairo_image_surface_get_stride(cairo_blend_surface)/sizeof (uint32_t));

    auto get_alpha_from_text_layer = [&](uint32_t row, uint32_t col) -> double {
        if(row < 0 || row >= height || col < 0 || col >= width)
            return 0;
        else
            return double(text_bitmap[row*text_stride + col] >> 24) / 255;
    };

    /*
    for(uint32_t i = 0; i < height; i++)
        for(uint32_t j = 0; j < width; j++) {
            double sum = 0;
            for(int32_t ki = -int32_t(config::shadow_radius); ki <= config::shadow_radius; ki++)
                for(int32_t kj = -int32_t(config::shadow_radius); kj <= config::shadow_radius; kj++)
                    sum += get_alpha_from_text_layer(i+ki, j+kj) * get_blur_kernel(config::shadow_radius, ki, kj);
            blend_bitmap[i*blend_stride + j] = uint32_t(sum*255) << 24;
        }
    */

    cairo_surface_mark_dirty(cairo_blend_surface);
    cairo_set_source_surface(cairo_blend_layer, cairo_text_surface, 0, 0);
    cairo_paint(cairo_blend_layer);
}

void CairoRendererPrivate::generate_blur_kernel(uint32_t radius) {
    blur_kernel.resize((radius+1)*(radius+1));
    // Make a working box blur
    for(double &i : blur_kernel)
        i = 1/((radius*2+1)*(radius*2+1));
}

double CairoRendererPrivate::get_blur_kernel(uint32_t radius, int32_t x, int32_t y) {
    uint32_t abs_x = uint32_t(std::abs(x));
    uint32_t abs_y = uint32_t(std::abs(y));
    if(abs_x > radius || abs_y > radius)
        return 0;
    else
        return blur_kernel[abs_x*(radius+1) + abs_y];
}

}
