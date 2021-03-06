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
namespace config {

extern const char *const config_filename;

extern uint32_t stage_width;
extern uint32_t extra_line_height;

extern const char *font_file;
extern uint32_t font_file_index;
extern double font_size;
extern double shadow_radius;

extern double danmaku_lifetime;
extern double danmaku_attack;
extern double danmaku_decay;

extern uint32_t max_fps;

}
}
