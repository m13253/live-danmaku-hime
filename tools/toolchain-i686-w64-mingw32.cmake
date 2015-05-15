# CMake toolchain configuration for Linux -> MinGW32 (Win32) cross compilation
# This can be enabled by ./configure -DCMAKE_TOOLCHAIN_FILE=tools/toolchain-i686-w64-mingw32.cmake
# The i686-w64-mingw32 toolchain is available at http://mingw-w64.sourceforge.net/

SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_SYSTEM_PROCESSOR i686-w64-mingw32)
SET(CMAKE_C_COMPILER ${CMAKE_SYSTEM_PROCESSOR}-gcc)
SET(CMAKE_CXX_COMPILER ${CMAKE_SYSTEM_PROCESSOR}-g++)
SET(CMAKE_RC_COMPILER ${CMAKE_SYSTEM_PROCESSOR}-windres)
SET(CMAKE_AR ${CMAKE_SYSTEM_PROCESSOR}-gcc-ar CACHE FILEPATH "Archiver")
SET(CMAKE_NM ${CMAKE_SYSTEM_PROCESSOR}-gcc-nm)
SET(CMAKE_RANLIB ${CMAKE_SYSTEM_PROCESSOR}-gcc-ranlib)
SET(CMAKE_STRIP ${CMAKE_SYSTEM_PROCESSOR}-strip)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# I will simply set -static for static linking, to get around MinGW CRT problem
# Alternatively, you can use the method here:
# http://m13253.blogspot.com/2015/01/mingw-w64-libwinpthread-1-dll-issue.html
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -static)
