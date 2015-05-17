.PHONY: all clean distclean

HOSTARCH=i686-w64-mingw32
AR:=$(HOSTARCH)-gcc-ar
CC:=$(HOSTARCH)-gcc
CXX:=$(HOSTARCH)-g++
NM:=$(HOSTARCH)-gcc-nm
RANLIB:=$(HOSTARCH)-gcc-ranlib
RC:=$(HOSTARCH)-windres
STRIP:=$(HOSTARCH)-strip
override startdir:=$(PWD)
export CFLAGS:=$(CFLAGS) -I$(startdir)/include -L$(startdir)/lib -O2
export CXXFLAGS:=$(CXXFLAGS) -I$(startdir)/include -L$(startdir)/lib -O2
export CPPFLAGS:=$(CPPFLAGS) -I$(startdir)/include
export LDFLAGS:=$(LDFLAGS) -L$(startdir)/lib -L.
export PKG_CONFIG_PATH:=$(startdir)/lib/pkgconfig

all: lib/libpng.a

clean:
	rm -rfv flag build lib include

distclean:
	rm -rfv flag src build lib include

flag:
	mkdir -p build lib include
	touch flag

src/pango.tar.xz:
	./stable_fetch $@ http://ftp.gnome.org/pub/GNOME/sources/pango/1.36/pango-1.36.8.tar.xz

src/cairo.tar.xz:
	./stable_fetch $@ http://cairographics.org/releases/cairo-1.14.2.tar.xz

src/pixman.tar.gz:
	./stable_fetch $@ http://cairographics.org/releases/pixman-0.32.6.tar.gz

src/harfbuzz.tar.bz2:
	./stable_fetch $@ http://www.freedesktop.org/software/harfbuzz/release/harfbuzz-0.9.40.tar.bz2

src/freetype.tar.bz2:
	./stable_fetch $@ http://download.savannah.gnu.org/releases/freetype/freetype-2.5.5.tar.bz2

src/libpng.tar.xz:
	./stable_fetch $@ http://download.sourceforge.net/libpng/libpng-1.6.17.tar.xz -U Wget/1

src/zlib.tar.xz:
	./stable_fetch $@ http://zlib.net/zlib-1.2.8.tar.xz

src/bzip2.tar.gz:
	./stable_fetch $@ http://www.bzip.org/1.0.6/bzip2-1.0.6.tar.gz

lib/libpango.a: src/pango.tar.xz flag lib/libcairo.a lib/libharfbuzz.a lib/libfreetype.a.2pass
	rm -rfv build/pango
	mkdir -p build/pango
	tar xJf $< -C build/pango

lib/libcairo.a: src/cairo.tar.xz flag lib/libpixman.a lib/libpng.a lib/libfreetype.a.2pass
	rm -rfv build/cairo
	mkdir -p build/cairo
	tar xJf $< -C build/cairo
	(cd build/cairo/cairo-* && ./configure --prefix "$(startdir)" --host "$(HOSTARCH)" --disable-shared --enable-static --disable-xlib --disable-xlib-xrender --disable-xcb --disable-xcb-shm --disable-quartz --disable-quartz-font --disable-win32 --disable-win32-font --disable-egl --disable-glx --disable-wgl --enable-ft --disable-fc --disable-ps --disable-pdf --disable-svg)
	$(MAKE) -C build/cairo/cairo-*
	$(MAKE) -C build/cairo/cairo-* install

lib/libpixman.a: src/pixman.tar.gz flag lib/libpng.a
	rm -rfv build/pixman
	mkdir -p build/pixman
	tar xzf $< -C build/pixman
	(cd build/pixman/pixman-* && ./configure --prefix "$(startdir)" --host "$(HOSTARCH)" --disable-shared --enable-static --enable-libpng)
	$(MAKE) -C build/pixman/pixman-*
	$(MAKE) -C build/pixman/pixman-* install

lib/libfreetype.a.2pass: src/freetype.tar.bz2 flag lib/libharfbuzz.a lib/libz.a lib/libbz2.a lib/libpng.a
	rm -rfv build/freetype
	mkdir -p build/freetype
	tar xjf $< -C build/freetype
	(cd build/freetype/freetype-* && ./configure --prefix "$(startdir)" --host "$(HOSTARCH)" --disable-shared --enable-static --with-harfbuzz)
	$(MAKE) -C build/freetype/freetype-*
	$(MAKE) -C build/freetype/freetype-* install
	sed -i -e 's/Libs:\(.*\)$/Libs:\1 -lharfbuzz/' lib/pkgconfig/freetype2.pc
	touch $@

lib/libharfbuzz.a: src/harfbuzz.tar.bz2 flag lib/libfreetype.a.1pass
	rm -rfv build/harfbuzz
	mkdir -p build/harfbuzz
	tar xjf $< -C build/harfbuzz
	(cd build/harfbuzz/harfbuzz-* && ./configure --prefix "$(startdir)" --host "$(HOSTARCH)" --disable-shared --enable-static --without-glib --without-gobject --without-cairo --without-icu --without-graphite2 --with-freetype)
	$(MAKE) -C build/harfbuzz/harfbuzz-*
	$(MAKE) -C build/harfbuzz/harfbuzz-* install

lib/libfreetype.a.1pass: src/freetype.tar.bz2 flag lib/libz.a lib/libbz2.a lib/libpng.a
	rm -rfv build/freetype
	mkdir -p build/freetype
	tar xjf $< -C build/freetype
	(cd build/freetype/freetype-* && ./configure --prefix "$(startdir)" --host "$(HOSTARCH)" --disable-shared --enable-static --without-harfbuzz)
	$(MAKE) -C build/freetype/freetype-*
	$(MAKE) -C build/freetype/freetype-* install
	touch $@

lib/libpng.a: src/libpng.tar.xz flag lib/libz.a
	rm -rfv build/libpng
	mkdir -p build/libpng
	tar xJf $< -C build/libpng
	(cd build/libpng/libpng-* && ./configure --prefix "$(startdir)" --host "$(HOSTARCH)" --disable-shared --enable-static)
	$(MAKE) -C build/libpng/libpng-*
	$(MAKE) -C build/libpng/libpng-* install


lib/libz.a: src/zlib.tar.xz flag
	rm -rfv build/zlib
	mkdir -p build/zlib
	tar xJf $< -C build/zlib
	$(MAKE) -C build/zlib/zlib-* -f win32/Makefile.gcc SHARED_MODE=0 prefix="$(startdir)" DESTDIR="$(startdir)" LIBRARY_PATH="/lib" INCLUDE_PATH="/include" AR="$(AR)" CC="$(CC)" RC="$(RC)" STRIP="$(STRIP)"
	$(MAKE) -C build/zlib/zlib-* -f win32/Makefile.gcc install SHARED_MODE=0 prefix="$(startdir)" DESTDIR="$(startdir)" LIBRARY_PATH="/lib" INCLUDE_PATH="/include" AR="$(AR)" CC="$(CC)" RC="$(RC)" STRIP="$(STRIP)"

lib/libbz2.a: src/bzip2.tar.gz flag
	rm -rfv build/bzip2
	mkdir -p build/bzip2
	tar xzf $< -C build/bzip2
	$(MAKE) -C build/bzip2/bzip2-* libbz2.a AR="$(AR)" CC="$(CC)" RANLIB="$(RANLIB)"
	install build/bzip2/bzip2-*/libbz2.a $@
	cp build/bzip2/bzip2-*/bzlib.h include/bzlib.h
