lib_LIBRARIES = libulapi.a

SOURCES = ../src/unix_ulapi.c ../src/ulapi.h ../src/inifile.c ../src/inifile.h ../src/unix_rtapi.c ../src/rtapi.h

libulapi_a_SOURCES = $(SOURCES)
libulapi_a_CFLAGS = 
if HAVE_RTAI
libulapi_a_CFLAGS += @RTAI_INCLUDE@
endif
if HAVE_IOPL
libulapi_a_CFLAGS += -O2
endif

if NO_DL
else

lib_LIBRARIES += libdlulapi.a
libdlulapi_a_SOURCES = $(SOURCES)
libdlulapi_a_CFLAGS = @DL_CFLAGS@
if HAVE_RTAI
libulapi_a_CFLAGS += @RTAI_INCLUDE@
endif
if HAVE_IOPL
libdlulapi_a_CFLAGS += -O2
endif

endif

EXTRA_DIST = CMakeLists.txt Makefile.am
