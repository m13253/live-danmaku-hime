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

    cairo_set_line_width(p->cairo_instance, 10.0);
    cairo_arc(p->cairo_instance, 80, 80, 32, 0, 3.14159265358979323846);
    cairo_stroke(p->cairo_instance);

    callback(reinterpret_cast<uint32_t *>(cairo_image_surface_get_data(p->cairo_surface)), uint32_t(cairo_image_surface_get_stride(p->cairo_surface)/sizeof (uint32_t)));
}

}
