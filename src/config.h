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

#pragma once

#include <cstdint>

namespace dmhm {
namespace config{

const uint32_t stage_width = 480;
const uint32_t extra_line_height = 8;

const char *const font_file = "font.ttf";
const uint32_t font_file_index = 0;
const double font_size = 24;
const double shadow_radius = 24;

const double danmaku_lifetime = 5;
const double danmaku_attack = 0.5;
const double danmaku_decay = 1;

}
}
