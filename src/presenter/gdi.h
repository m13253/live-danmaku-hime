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
#include "../utils.h"
#include <cstdint>
#include <string>

namespace dmhm {

class GDIPresenter {

public:

    GDIPresenter(class Application *app);
    ~GDIPresenter();
    void report_error(const std::string error);
    void get_stage_size(uint32_t &width, uint32_t &height);
    void paint_frame();
    int run_loop();

private:

    proxy_ptr<struct GDIPresenterPrivate> p;

};

}
