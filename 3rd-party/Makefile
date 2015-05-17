.PHONY: all clean distclean

HOSTARCH=i686-w64-mingw32
AR:=$(HOSTARCH)-gcc-ar
CC:=$(HOSTARCH)-gcc
CXX:=$(HOSTARCH)-g++
NM:=$(HOSTARCH)-gcc-nm
RANLIB:=$(HOSTARCH)-gcc-ranlib
WGET:=wget -c --no-use-server-timestamps
override CFLAGS:=$(CFLAGS) -I$(PWD)/include -L$(PWD)/lib -O2
override CXXFLAGS:=$(CXXFLAGS) -I$(PWD)/include -L$(PWD)/lib -O2
override CPPFLAGS:=$(CPPFLAGS) -I$(PWD)/include
override LDFLAGS:=$(LDFLAGS) -L$(PWD)/lib
MAKEFLAGS:=-j$(shell nproc || echo 1)

all: lib/libbz2.a

clean:
	rm -rfv flag build lib include

distclean:
	rm -rfv flag src build lib include

flag:
	mkdir -p src build lib include
	touch flag

src/pango.tar.xz: flag
	$(WGET) -O $@ http://ftp.gnome.org/pub/GNOME/sources/pango/1.36/pango-1.36.8.tar.xz

src/cairo.tar.xz: flag
	$(WGET) -O $@ http://cairographics.org/releases/cairo-1.14.2.tar.xz

src/harfbuzz.tar.bz2: flag
	$(WGET) -O $@ http://www.freedesktop.org/software/harfbuzz/release/harfbuzz-0.9.40.tar.bz2

src/freetype2.tar.bz2: flag
	$(WGET) -O $@ http://download.savannah.gnu.org/releases/freetype/freetype-2.5.5.tar.bz2

src/libpng.tar.xz: flag
	$(WGET) -O $@ http://download.sourceforge.net/libpng/libpng-1.6.17.tar.xz

src/zlib.tar.xz: flag
	$(WGET) -O $@ http://zlib.net/zlib-1.2.8.tar.xz

src/bzip2.tar.gz: flag
	$(WGET) -O $@ http://www.bzip.org/1.0.6/bzip2-1.0.6.tar.gz

lib/libpango.a: src/pango.tar.xz flag lib/libcairo.a
	rm -rfv build/pango
	mkdir -p build/pango
	tar -C build/pango

lib/libbz2.a: src/bzip2.tar.gz flag
	rm -rfv build/bzip2
	mkdir -p build/bzip2
	tar xzf $< -C build/bzip2
	$(MAKE) -C build/bzip2/bzip2-* libbz2.a AR="$(AR)" CC="$(CC)" RANLIB="$(RANLIB)"
	install build/bzip2/bzip2-*/libbz2.a $@
	cp build/bzip2/bzip2-*/bzlib.h include/bzlib.h
