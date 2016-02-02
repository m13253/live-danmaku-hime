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

#include "config.h"
#include "utils.h"
#include <cstring>
#include <confuse.h>

namespace dmhm {

static char str_stage_width[] = "stage_width";
static char str_extra_line_height[] = "extra_line_height";
static char str_font_file[] = "font_file";
static char str_font_file_index[] = "font_file_index";
static char str_font_size[] = "font_size";
static char str_shadow_radius[] = "shadow_radius";
static char str_danmaku_lifetime[] = "danmaku_lifetime";
static char str_danmaku_attack[] = "danmaku_attack";
static char str_danmaku_decay[] = "danmaku_decay";
static char str_max_fps[] = "max_fps";

int load_config(const char *config_filename) {
    long int stage_width = config::stage_width;
    long int extra_line_height = config::extra_line_height;
    char *font_file = strdup(config::font_file);
    long int font_file_index = config::font_file_index;
    double font_size = config::font_size;
    double shadow_radius = config::shadow_radius;
    double danmaku_lifetime = config::danmaku_lifetime;
    double danmaku_attack = config::danmaku_attack;
    double danmaku_decay = config::danmaku_decay;
    long int max_fps = config::max_fps;

    cfg_opt_t opts[] = {
        CFG_SIMPLE_INT(str_stage_width, &stage_width),
        CFG_SIMPLE_INT(str_extra_line_height, &extra_line_height),
        CFG_SIMPLE_STR(str_font_file, &font_file),
        CFG_SIMPLE_INT(str_font_file_index, &font_file_index),
        CFG_SIMPLE_FLOAT(str_font_size, &font_size),
        CFG_SIMPLE_FLOAT(str_shadow_radius, &shadow_radius),
        CFG_SIMPLE_FLOAT(str_danmaku_lifetime, &danmaku_lifetime),
        CFG_SIMPLE_FLOAT(str_danmaku_attack, &danmaku_attack),
        CFG_SIMPLE_FLOAT(str_danmaku_decay, &danmaku_decay),
        CFG_SIMPLE_INT(str_max_fps, &max_fps),
        CFG_END()
    };
    cfg_t *cfg = cfg_init(opts, 0);
    int parse_result = cfg_parse(cfg, config_filename);
    cfg_free(cfg);

    dmhm_assert(parse_result != CFG_FILE_ERROR);
    dmhm_assert(parse_result != CFG_PARSE_ERROR);
    dmhm_assert(parse_result == CFG_SUCCESS);

    dmhm_assert(stage_width > 0);
    dmhm_assert(extra_line_height >= 0);
    dmhm_assert(font_file != nullptr);
    dmhm_assert(*font_file != '\0');
    dmhm_assert(font_file_index >= 0);
    dmhm_assert(font_size >= 0);
    dmhm_assert(shadow_radius >= 0 && shadow_radius <= stage_width);
    dmhm_assert(danmaku_lifetime > 0);
    dmhm_assert(danmaku_attack >= 0);
    dmhm_assert(danmaku_decay >= 0);
    dmhm_assert(danmaku_attack + danmaku_decay <= danmaku_lifetime);

    config::stage_width = stage_width;
    config::extra_line_height = extra_line_height;
    config::font_file = font_file;
    config::font_file_index = font_file_index;
    config::font_size = font_size;
    config::shadow_radius = shadow_radius;
    config::danmaku_lifetime = danmaku_lifetime;
    config::danmaku_attack = danmaku_attack;
    config::danmaku_decay = danmaku_decay;
    config::max_fps = max_fps;

    return parse_result;
}

}
