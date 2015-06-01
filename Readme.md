Live Danmaku Hime
=================

This is a desktop widget to display live comments for various live video broadcast website.

It hangs on the right side of your desktop and displays comments from your audience when you broadcast.

## Compiling for Windows/GDI+ backend

You need [MSYS2](https://msys2.github.io/) or Linux, with [i686-w64-mingw32 cross compiler](http://mingw-w64.org/) and [CMake](http://www.cmake.org/) installed.

First go to the directory `3rd-party`, type `make`. This will download and compile third party libraries including [Cairo](http://www.cairographics.org/), [FreeType](http://www.freetype.org/) and [libconfuse](http://www.nongnu.org/confuse/).

Then back to the main directory, type `./configure -DCMAKE_TOOLCHAIN_FILE=tools/toolchain-i686-w64-mingw32.cmake` and `make`.

There are reports of success build with Microsoft VC++ compiler, but they are not tested and need some hack.

## Compiling for Linux/Gtk+ backend

_Still under development._

## Getting ready for running

1. Choose a frontend according to which broadcast website you are using.

2. Edit `live_danmaku_hime.conf` to fit your need.

3. Choose a font, for Simplified Chinese I recommend [思源黑体 Source Han Sans](https://github.com/adobe-fonts/source-han-sans/tree/release/OTF/SimplifiedChinese). Name it `font.ttf`.

4. Put `live_danmaku_hime.exe`, `live_danmaku_hime.conf`, `font.ttf` altogether in the same directory with the frontend.

5. Start the frontend.

## Choose a frontend

For the following websites, choose the frontends below:

- [sina-live-fetcher](https://github.com/m13253/sina-live-fetcher/tree/master/live-comment-fetcher) written in Go for [kan.sina.com.cn](http://kan.sina.com.cn/)

- David Huang's [SinaDanmakuHime](https://github.com/hjc4869/SinaDanmakuHime.GUI) written in C# for [kan.sina.com.cn](http://kan.sina.com.cn/)

- _Still under development_

## Write a frontend

In order to fetch comments from various websites, different frontend programs need to be used.

Your frontend should execute `live_danmaku_hime.exe` upon execution, fetch comments from live chat, then feed plain UTF-8 strings, separated with `\n`, to `stdin` of `live_danmaku_hime.exe`.

Please do not forget to `flush` after writing a line, do not forget to consume `stderr` of `live_danmaku_hime.exe` to prevent buffer full.

## Internationalization

This program was originally written for kan.sina.com.cn, so no internationalization was intended. Strings and error messages were hard-coded in Simplified Chinese.

However, since this program is open-source, you can add internationalization yourself and kindly send me a pull request.

## Missing functionalities

- Complex text layout and shaping with Pango or Harfbuzz.

- Hardware accelerated rendering with OpenGL or SIMD CPU instructions.

- More backends.

## Licensing information

This program is licensed under BSD license. See [COPYING](COPYING) for more information.
