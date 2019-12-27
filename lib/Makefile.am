AM_CPPFLAGS = -I../src

# build Unix ulapi and rtapi no matter what else is installed

lib_LIBRARIES = libunixulapi.a libunixrtapi.a

libunixulapi_a_SOURCES = ../src/unix_ulapi.c ../src/ulapi.h ../src/inifile.c ../src/inifile.h
libunixulapi_a_CFLAGS = -DTARGET_UNIX @PIC_CFLAGS@

libunixrtapi_a_SOURCES = ../src/unix_rtapi.c ../src/rtapi.h
libunixrtapi_a_CFLAGS = -DTARGET_UNIX @PIC_CFLAGS@

if HAVE_IOPL
libunixulapi_a_CFLAGS += -O2
libunixrtapi_a_CFLAGS += -O2
endif

# build Xenomai ulapi and rtapi if it's installed

if HAVE_XENOMAI

lib_LIBRARIES += libxenoulapi.a libxenortapi.a

libxenoulapi_a_SOURCES = ../src/xeno_ulapi.c ../src/ulapi.h
libxenoulapi_a_CFLAGS = -DTARGET_XENOMAI @XENOMAI_CFLAGS@ @PIC_CFLAGS@

libxenortapi_a_SOURCES = ../src/xeno_rtapi.c ../src/rtapi.h
libxenortapi_a_CFLAGS = -DTARGET_XENOMAI @XENOMAI_CFLAGS@ @PIC_CFLAGS@

if HAVE_IOPL
libxenoulapi_a_CFLAGS += -O2
libxenortapi_a_CFLAGS += -O2
endif

endif

EXTRA_DIST = CMakeLists.txt Makefile.am
