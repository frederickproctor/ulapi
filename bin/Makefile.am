AM_CPPFLAGS = -I../src

bin_PROGRAMS = ultest semtest mutextest inifind sockettest serialtest broadcasttest multicasttest inb outb

ultest_SOURCES = ../src/ultest.c
ultest_CFLAGS = -DTARGET_UNIX
ultest_LDADD = -L../lib -lulapi @PTHREAD_LIBS@ @RTAI_LIBS@
ultest_DEPENDENCIES = ../lib/libunixulapi.a

semtest_SOURCES = ../src/semtest.c
semtest_CFLAGS = -DTARGET_UNIX
semtest_LDADD = -L../lib -lulapi @PTHREAD_LIBS@ @RTAI_LIBS@
semtest_DEPENDENCIES = ../lib/libunixulapi.a

mutextest_SOURCES = ../src/mutextest.c
mutextest_CFLAGS = -DTARGET_UNIX
mutextest_LDADD = -L../lib -lulapi @PTHREAD_LIBS@ @RTAI_LIBS@
mutextest_DEPENDENCIES = ../lib/libunixulapi.a

serialtest_SOURCES = ../src/serialtest.c
serialtest_CFLAGS = -DTARGET_UNIX
serialtest_LDADD = -L../lib -lulapi @PTHREAD_LIBS@ @RTAI_LIBS@
serialtest_DEPENDENCIES = ../lib/libunixulapi.a

inifind_SOURCES = ../src/inifile.c ../src/inifile.h
inifind_CFLAGS = -DTARGET_UNIX
inifind_CFLAGS += -DINIFILE_MAIN

sockettest_SOURCES = ../src/sockettest.c
sockettest_CFLAGS = -DTARGET_UNIX
sockettest_LDADD = -L../lib -lulapi @PTHREAD_LIBS@ @RTAI_LIBS@
sockettest_DEPENDENCIES = ../lib/libunixulapi.a

broadcasttest_SOURCES = ../src/broadcasttest.c
broadcasttest_CFLAGS = -DTARGET_UNIX
broadcasttest_LDADD = -L../lib -lulapi @PTHREAD_LIBS@ @RTAI_LIBS@
broadcasttest_DEPENDENCIES = ../lib/libunixulapi.a

multicasttest_SOURCES = ../src/multicasttest.c
multicasttest_CFLAGS = -DTARGET_UNIX
multicasttest_LDADD = -L../lib -lulapi @PTHREAD_LIBS@ @RTAI_LIBS@
multicasttest_DEPENDENCIES = ../lib/libunixulapi.a

inb_SOURCES = ../src/inb.c
inb_CFLAGS = -DTARGET_UNIX
inb_CFLAGS += -O2
inb_LDADD = -L../lib -lunixrtapi -lunixulapi @PTHREAD_LIBS@
inb_DEPENDENCIES = ../lib/libunixrtapi.a

outb_SOURCES = ../src/outb.c
outb_CFLAGS = -DTARGET_UNIX
outb_CFLAGS += -O2
outb_LDADD = -L../lib -lunixrtapi -lunixulapi @PTHREAD_LIBS@
outb_DEPENDENCIES = ../lib/libunixrtapi.a

if HAVE_XENOMAI

bin_PROGRAMS += rt_timer_test ul_timer_test

rt_timer_test_SOURCES = ../src/rt_timer_test.c
rt_timer_test_CFLAGS = -DTARGET_XENOMAI @XENOMAI_CFLAGS@
rt_timer_test_LDADD = -L../lib -lxenortapi @XENOMAI_LDFLAGS@ -lm

ul_timer_test_SOURCES = ../src/ul_timer_test.c
ul_timer_test_CFLAGS = -DTARGET_UNIX
ul_timer_test_LDADD = -L../lib -lunixrtapi -lunixulapi @PTHREAD_LIBS@

endif

EXTRA_DIST = CMakeLists.txt Makefile.am
