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

#include "danmaku_entry.h"
#include "../config.h"
#include "../utils.h"
#include <chrono>
#include <string>

namespace dmhm {

struct DanmakuEntryPrivate {
    std::chrono::steady_clock::time_point timestamp;
    std::string message;
};

DanmakuEntry::DanmakuEntry(const std::string &message) {
    p->timestamp = std::chrono::steady_clock::now();
    p->message = std::move(message);
}

DanmakuEntry::~DanmakuEntry() {
}

DanmakuEntry::DanmakuEntry(const DanmakuEntry &other) :
    p(other.p) {
}

DanmakuEntry::DanmakuEntry(DanmakuEntry &&other) :
    p(std::move(other.p)) {
}

DanmakuEntry &DanmakuEntry::set_message(const std::string &message) {
    p->message = std::move(message);
    return *this;
}

std::string DanmakuEntry::get_message() const {
    return p->message;
}

std::chrono::steady_clock::time_point DanmakuEntry::get_timestamp() const {
    return p->timestamp;
}

}
