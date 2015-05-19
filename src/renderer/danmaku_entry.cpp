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
#include "../utils.h"
#include <chrono>
#include <string>

namespace dmhm {

DanmakuEntry::DanmakuEntry(const std::string &message) :
    message(std::move(message)),
    timestamp(std::chrono::steady_clock::now()) {
}

DanmakuEntry::DanmakuEntry(const DanmakuEntry &other) :
    message(other.message),
    timestamp(other.timestamp) {
}

DanmakuEntry::DanmakuEntry(DanmakuEntry &&other) {
    std::swap(message, other.message);
    std::swap(timestamp, other.timestamp);
}

}
