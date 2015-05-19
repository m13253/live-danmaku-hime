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
#include <functional>
#include <list>
#include <cairo/cairo.h>

namespace dmhm {

struct CairoRendererPrivate {
    Application *app = nullptr;
    cairo_surface_t *cairo_surface = nullptr;
    cairo_t *cairo_instance = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
};

CairoRenderer::CairoRenderer(Application *app) {
    p->app = app;
}

CairoRenderer::~CairoRenderer() {
    if(p->cairo_instance) {
        cairo_destroy(p->cairo_instance);
        p->cairo_instance = nullptr;
    }
    if(p->cairo_surface) {
        cairo_surface_destroy(p->cairo_surface);
        p->cairo_surface = nullptr;
    }
}

void CairoRenderer::paint_frame(uint32_t width, uint32_t height, std::function<void (const uint32_t *bitmap, uint32_t stride)> callback) {
    if(width != p->width || height != p->height) {
        p->width = width;
        p->height = height;
        if(p->cairo_instance) {
            cairo_destroy(p->cairo_instance);
            p->cairo_instance = nullptr;
        }
        if(p->cairo_surface) {
            cairo_surface_destroy(p->cairo_surface);
            p->cairo_surface = nullptr;
        }
    }
    if(!p->cairo_surface)
        p->cairo_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    if(!p->cairo_instance)
        p->cairo_instance = cairo_create(p->cairo_surface);

    cairo_save(p->cairo_instance);
    cairo_set_operator(p->cairo_instance, CAIRO_OPERATOR_CLEAR);
    cairo_paint(p->cairo_instance);
    cairo_restore(p->cairo_instance);

    static uint32_t counter = 0;
    if(counter == 0) counter = height+48;
    cairo_set_source_rgba(p->cairo_instance, 0.4, 0.8, 1.0, 0.6);
    cairo_select_font_face(p->cairo_instance, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(p->cairo_instance, 48);
    cairo_move_to(p->cairo_instance, 80, counter--);
    cairo_show_text(p->cairo_instance, "Lorem ipsum dolor sit amet.");
    cairo_select_font_face(p->cairo_instance, "Microsoft Yahei UI", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_move_to(p->cairo_instance, 80, counter-48);
    cairo_show_text(p->cairo_instance, "\xe5\xa4\xa9\xe5\x9c\xb0\xe7\x8e\x84\xe9\xbb\x84\xef\xbc\x8c\xe5\xae\x87\xe5\xae\x99\xe6\xb4\xaa\xe8\x8d\x92\xe3\x80\x82");
    cairo_stroke(p->cairo_instance);

    callback(reinterpret_cast<uint32_t *>(cairo_image_surface_get_data(p->cairo_surface)), uint32_t(cairo_image_surface_get_stride(p->cairo_surface)/sizeof (uint32_t)));
}

}
