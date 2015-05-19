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

#include "console.h"
#include "../utils.h"
#include "../app.h"
#include <iostream>
#include <string>
#include <thread>

namespace dmhm {

struct ConsoleFetcherPrivate {
    Application *app = nullptr;
    std::thread thread;
    void do_run(ConsoleFetcher *pub);
};

ConsoleFetcher::ConsoleFetcher(Application *app) {
    p->app = app;
}

ConsoleFetcher::~ConsoleFetcher() {
}

void ConsoleFetcher::run_thread() {
    p->thread = std::thread([&]() {
        p->do_run(this);
    });
}

void ConsoleFetcherPrivate::do_run(ConsoleFetcher *pub) {
    std::string input_buffer;
    while(std::getline(std::cin, input_buffer)) {
        std::cout << input_buffer << std::endl;
    }
}

}
