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
#include <memory>
#include <gtkmm.h>

namespace dmhm {

struct GtkPresenterPrivate {
    Glib::RefPtr<Gtk::Application> gtkapp;
    std::unique_ptr<Gtk::Window> window;
    int32_t top; int32_t left; int32_t right; int32_t bottom;
    void get_stage_rect(GtkPresenter *pub);
};

GtkPresenter::GtkPresenter(Application *app) {
    p->gtkapp = Gtk::Application::create("com.starbrilliant.live-danmaku-hime");
    p->get_stage_rect(this);
    p->window.reset(new Gtk::Window());
    p->window->set_accept_focus(false);
    p->window->set_app_paintable(true);
    p->window->set_decorated(false);
    p->window->set_focus_on_map(false);
    p->window->set_keep_above(true);
    p->window->set_skip_pager_hint(true);
    p->window->set_skip_taskbar_hint(true);
    Gdk::RGBA transparent;
    transparent.set_rgba(0, 0, 0, 1);
    p->window->override_background_color(transparent);
    p->window->move(p->left, p->top);
    p->window->resize(p->right - p->left, p->bottom - p->top);
    p->window->show();
}

GtkPresenter::~GtkPresenter() {
}

void GtkPresenter::report_error(const std::string error) {
    Gtk::MessageDialog(error, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
}

void GtkPresenter::get_stage_size(uint32_t &width, uint32_t &height) {
    dmhm_assert(p->right >= p->left && p->bottom >= p->top);
    width = p->right - p->left;
    height = p->bottom - p->top;
}

void GtkPresenter::paint_frame() {
    report_error("Unimplemented");
}

int GtkPresenter::run_loop() {
    p->gtkapp->run(*p->window);
    return 0;
}

void GtkPresenterPrivate::get_stage_rect(GtkPresenter *pub) {
    Glib::RefPtr<Gdk::Screen> screen = Gdk::Screen::get_default();
    Gdk::Rectangle rect = screen->get_monitor_workarea();
    top = rect.get_y();
    left = rect.get_x() + rect.get_width() - config::stage_width;
    right = rect.get_x() + rect.get_width();
    bottom = rect.get_y() + rect.get_height();
}

}
