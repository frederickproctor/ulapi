AM_CPPFLAGS = -I../src

SOURCES = ../src/unix_ulapi.c ../src/ulapi.h ../src/inifile.c ../src/inifile.h ../src/unix_rtapi.c ../src/rtapi.h

# build Unix ulapi and rtapi no matter what else is installed

lib_LIBRARIES = libunixulapi.a libunixrtapi.a

libunixulapi_a_SOURCES = ../src/unix_ulapi.c ../src/ulapi.h ../src/inifile.c ../src/inifile.h
libunixulapi_a_CFLAGS = -DTARGET_UNIX

libunixrtapi_a_SOURCES = ../src/unix_rtapi.c ../src/rtapi.h
libunixrtapi_a_CFLAGS = -DTARGET_UNIX

if HAVE_IOPL
libunixulapi_a_CFLAGS += -O2
endif

# build Xenomai ulapi and rtapi if it's installed

if HAVE_XENOMAI

lib_LIBRARIES += libxenortapi.a

libxenortapi_a_SOURCES = ../src/xeno_rtapi.c ../src/rtapi.h
libxenortapi_a_CFLAGS = -DTARGET_XENOMAI @XENOMAI_CFLAGS@

endif

EXTRA_DIST = CMakeLists.txt Makefile.am
