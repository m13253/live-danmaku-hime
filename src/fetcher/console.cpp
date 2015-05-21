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
#include "../renderer/danmaku_entry.h"
#include <atomic>
#include <functional>
#include <iostream>
#include <list>
#include <mutex>
#include <string>
#include <thread>

namespace dmhm {

struct ConsoleFetcherPrivate {
    Application *app = nullptr;
    std::thread thread;
    std::atomic<bool> is_eof = {false};
    std::mutex mutex;
    std::list<DanmakuEntry> message_queue;
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

bool ConsoleFetcher::is_eof() {
    return p->is_eof;
}

void ConsoleFetcher::pop_messages(std::function<void (DanmakuEntry &entry)> callback) {
    std::unique_lock<std::mutex> lock(p->mutex);
    while(!p->message_queue.empty()) {
        callback(p->message_queue.front());
        p->message_queue.pop_front();
    }
}

void ConsoleFetcherPrivate::do_run(ConsoleFetcher *pub) {
    std::string input_buffer;
    while(std::getline(std::cin, input_buffer)) {
        std::unique_lock<std::mutex> lock(mutex);
        message_queue.push_back(DanmakuEntry(utf8_validify(input_buffer)));
    }
    is_eof = true;
}

}
