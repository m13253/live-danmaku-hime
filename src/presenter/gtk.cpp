/*
  Copyright (c) 2016 StarBrilliant <m13253@hotmail.com>
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

#include "gtk.h"
#include "../utils.h"
#include "../app.h"
#include "../renderer/renderer.h"
#include "../config.h"
#include <cstdlib>
#include <functional>
#include <memory>
#include <gtkmm.h>

#include <cstdio>

namespace dmhm {

struct GtkPresenterPrivate {
    class Window : public Gtk::Window {
    public:
        Window(GtkPresenter *pub) :
            Gtk::Window(Gtk::WINDOW_TOPLEVEL),
            pub(pub) {
        }
    protected:
        virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);
    private:
        GtkPresenter *pub;
    };

    Application *app = nullptr;
    Glib::RefPtr<Gtk::Application> gtkapp;
    std::unique_ptr<Window> window;
    int32_t top; int32_t left; int32_t right; int32_t bottom;
    void get_stage_rect(GtkPresenter *pub);
    void do_paint(GtkPresenter *pub, const Cairo::RefPtr<Cairo::Context> &cr, const uint32_t *bitmap, uint32_t width, uint32_t height, uint32_t stride);
};

GtkPresenter::GtkPresenter(Application *app) {
    p->app = app;
    p->gtkapp = Gtk::Application::create("com.starbrilliant.live-danmaku-hime");
    p->get_stage_rect(this);
    p->window.reset(new GtkPresenterPrivate::Window(this));

    p->window->set_title("\xe5\xbc\xb9\xe5\xb9\x95\xe5\xa7\xac");
    p->window->set_accept_focus(false);
    p->window->set_app_paintable(true);
    p->window->set_decorated(false);
    p->window->set_focus_on_map(false);
    p->window->set_keep_above(true);
    p->window->set_skip_pager_hint(true);
    p->window->set_skip_taskbar_hint(true);
    p->window->move(p->left, p->top);
    uint32_t width, height;
    get_stage_size(width, height);
    p->window->resize(width, height);

    Glib::RefPtr<Gdk::Screen> screen = p->window->get_screen();
    Glib::RefPtr<Gdk::Visual> visual = screen->get_rgba_visual();
    if(!visual) {
        /* Desktop compositor failed to set window transparency */
        report_error("\xe6\xa1\x8c\xe9\x9d\xa2\xe6\xb7\xb7\xe6\x88\x90\xe5\x99\xa8\xe6\x97\xa0\xe6\xb3\x95\xe8\xae\xbe\xe7\xbd\xae\xe9\x80\x8f\xe6\x98\x8e\xe7\xaa\x97\xe5\x8f\xa3");
        abort();
    }
    /* set_visual has no C++ binding, calling C function */
    gtk_widget_set_visual(GTK_WIDGET(p->window->gobj()), visual->gobj());

    p->window->show();
    Glib::signal_timeout().connect([&]() -> bool {
        paint_frame();
        return true;
    }, 1000/config::max_fps);
}

GtkPresenter::~GtkPresenter() {
}

void GtkPresenter::report_error(const std::string error) {
    Gtk::MessageDialog dialog = Gtk::MessageDialog(error, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    dialog.show();
    dialog.signal_response().connect([&](int) {
        p->gtkapp->quit();
    });
    p->gtkapp->run(dialog);
    p->gtkapp->quit();
    abort();
}

void GtkPresenter::get_stage_size(uint32_t &width, uint32_t &height) {
    dmhm_assert(p->right >= p->left && p->bottom >= p->top);
    width = p->right - p->left;
    height = p->bottom - p->top;
}

void GtkPresenter::paint_frame() {
    p->window->queue_draw();
}

int GtkPresenter::run_loop() {
    p->gtkapp->run(*p->window);
    return 0;
}

void GtkPresenterPrivate::get_stage_rect(GtkPresenter *pub) {
    Glib::RefPtr<Gdk::Screen> screen = Gdk::Screen::get_default();
    if(!screen) {
        /* Failed to retrieve screen size */
        pub->report_error("\xe8\x8e\xb7\xe5\x8f\x96\xe5\xb1\x8f\xe5\xb9\x95\xe5\xb0\xba\xe5\xaf\xb8\xe5\xa4\xb1\xe8\xb4\xa5");
        abort();
    }
    Gdk::Rectangle rect = screen->get_monitor_workarea();
    top = rect.get_y();
    left = rect.get_x() + rect.get_width() - config::stage_width;
    right = rect.get_x() + rect.get_width();
    bottom = rect.get_y() + rect.get_height();
}

void GtkPresenterPrivate::do_paint(GtkPresenter *pub, const Cairo::RefPtr<Cairo::Context> &cr, const uint32_t *bitmap, uint32_t width, uint32_t height, uint32_t stride) {
    Cairo::RefPtr<Cairo::ImageSurface> surface = Cairo::ImageSurface::create(reinterpret_cast<unsigned char *>(const_cast<uint32_t *>(bitmap)), Cairo::FORMAT_ARGB32, width, height, stride*sizeof (uint32_t));
    cr->set_source(surface, 0, 0);
    cr->move_to(0, 0);
    cr->set_operator(Cairo::OPERATOR_SOURCE);
    cr->paint();
}

bool GtkPresenterPrivate::Window::on_draw(const Cairo::RefPtr<Cairo::Context> &cr) {
    Gtk::Window::on_draw(cr);

    uint32_t width, height;
    pub->get_stage_size(width, height);
    move(pub->p->left, pub->p->top);
    resize(width, height);

    Cairo::RectangleInt click_rect = { 0, 0, 1, 1 };
    input_shape_combine_region(Cairo::Region::create(click_rect));

    Renderer *renderer = reinterpret_cast<Renderer *>(pub->p->app->get_renderer());
    dmhm_assert(renderer);
    if(!renderer->paint_frame(width, height, [&](const uint32_t *bitmap, uint32_t stride) {
        pub->p->do_paint(pub, cr, bitmap, width, height, stride);
    }))
        pub->p->gtkapp->quit();

    return false;
}

}
