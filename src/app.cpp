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

#include "app.h"
#include "renderer/renderer.h"
#include "presenter/presenter.h"
#include <memory>

namespace dmhm {

struct ApplicationPrivate {
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Presenter> presenter;
};

Application::Application() {
    p->renderer.reset(new Renderer(this));
    p->presenter.reset(new Presenter(this));
}

Application::~Application() {
}

struct BaseFetcher *Application::get_fetcher() const {
//    return reinterpret_cast<struct BaseFetcher *>(p->fetcher.get());
    return nullptr;
}

struct BaseRenderer *Application::get_renderer() const {
    return reinterpret_cast<struct BaseRenderer *>(p->renderer.get());
}

struct BasePresenter *Application::get_presenter() const {
    return reinterpret_cast<struct BasePresenter *>(p->presenter.get());
}

int Application::run() {
    return p->presenter->run_loop();
}

}
