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
#include <cmath>
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

    std::unique_ptr<float []> blur_kernel;
    std::unique_ptr<float []> blur_temp;
    void generate_blur_kernel();
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

    p->generate_blur_kernel();
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
        p->blur_temp.reset(new float[width*height]);
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
    double alpha = 0;
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
            if(i.moving) {
                i.starty = i.starty+(i.endy-i.starty)*(now-i.starttime).count()/(i.endtime-i.starttime).count();
                i.endy -= animator.height;
            } else {
                i.starty = i.y;
                i.endy = i.starty-animator.height;
                i.moving = true;
            }
            i.starttime = now;
            i.endtime = now + std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(config::danmaku_attack));
        }
        danmaku_list.push_front(std::move(animator));
    });
}

void CairoRendererPrivate::animate_text(std::chrono::steady_clock::time_point now) {
    danmaku_list.remove_if([&](const DanmakuAnimator &x) -> bool {
        double timespan = double((now-x.entry.timestamp).count())*std::chrono::steady_clock::period::num/std::chrono::steady_clock::period::den;
        return timespan >= config::danmaku_lifetime || x.y < -2*config::shadow_radius;
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
    }
    cairo_stroke(cairo_text_layer);
}

void CairoRendererPrivate::blend_layers() {
    uint32_t radius = uint32_t(config::shadow_radius);

    cairo_surface_flush(cairo_text_surface);
    cairo_surface_flush(cairo_blend_surface);
    const uint32_t *text_bitmap = reinterpret_cast<uint32_t *>(cairo_image_surface_get_data(cairo_text_surface));
    uint32_t text_stride = uint32_t(cairo_image_surface_get_stride(cairo_text_surface)/sizeof (uint32_t));
    cairo_surface_flush(cairo_blend_surface);
    uint32_t *blend_bitmap = reinterpret_cast<uint32_t *>(cairo_image_surface_get_data(cairo_blend_surface));
    uint32_t blend_stride = uint32_t(cairo_image_surface_get_stride(cairo_blend_surface)/sizeof (uint32_t));

    for(uint32_t i = 0; i < width*height; i++)
        blur_temp[i] = 0.0;

    for(uint32_t y = 0; y < height; y++)
        for(uint32_t x = 0; x < width; x++) {
            uint32_t alpha = text_bitmap[y*text_stride + x];
            if(alpha != 0)
                for(uint32_t dx = 0; dx <= radius*2; dx++)
                    if(x+dx >= radius) {
                        uint32_t column = x+dx-radius;
                        if(column < width)
                            blur_temp[y*width + column] += alpha*blur_kernel[dx];
                    }
        }

    for(uint32_t x = 0; x < width; x++)
        for(uint32_t y = 0; y < height; y++) {
            float alpha = blur_temp[y*width + x];
            if(alpha != 0.0f) // It is safe to compare directly since no floating point error may occur
                for(uint32_t dy = 0; dy <= radius*2; dy++)
                    if(y+dy >= radius) {
                        uint32_t row = y+dy-radius;
                        if(row < height) {
                            uint32_t old_alpha = blend_bitmap[row*blend_stride + x];
                            old_alpha += uint32_t(blur_temp[y*width + x]*blur_kernel[dy]/0x100);
                            blend_bitmap[row*blend_stride + x] = std::min(old_alpha, 0xff0000u);
                        }
                    }
        }

    for(uint32_t i = 0; i < blend_stride*height; i++)
        blend_bitmap[i] = uint32_t((1-(1-blend_bitmap[i]/double(0xff0000))*(1-blend_bitmap[i]/double(0xff0000)))*0xff000000) & 0xff000000;

    cairo_surface_mark_dirty(cairo_blend_surface);
    cairo_set_source_surface(cairo_blend_layer, cairo_text_surface, 0, 0);
    cairo_paint(cairo_blend_layer);
}

void CairoRendererPrivate::generate_blur_kernel() {
    dmhm_assert(config::shadow_radius >= 0);
    int32_t radius = int32_t(config::shadow_radius);

    blur_kernel.reset(new float[radius*2+1]);
    float db_sq_sigma = radius*radius*2/9.0;
    float sum = 0;
    for(int32_t i = 0; i <= radius*2; i++)
        sum += blur_kernel[i] = exp(-(i-radius)*(i-radius)/db_sq_sigma);
    for(int32_t i = 0; i <= radius*2; i++)
        blur_kernel[i] /= sum;
}

}
