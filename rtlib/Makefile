# Get Linux kernel version major and minor, e.g., 2.6
UTS_RELEASE = $(shell uname -r | cut -f 1,2 -d .)

ifeq ($(UTS_RELEASE), 2.4)

ifdef MAKING_MODULES

EXTRA_CFLAGS += -I -I/usr/include -I/usr/realtime/include -ffast-math -mhard-float -D__IN_RTAI__ -DEXPORT_SYMTAB

obj-m += rtapi_hal_mod.o

rtapi_hal_mod-objs := rtai_rtapi.o rtai_rtapi_hal.o

include $(TOPDIR)/Rules.make

rtapi_hal_mod.o:  $(rtapi_hal_mod-objs)
	$(LD) $(LD_RFLAG) -r -o $@ $(rtapi_hal_mod-objs)

endif

clean :
	\rm -f *.o

else

# not 2.4, try 2.6

ifeq ($(UTS_RELEASE), 2.6)

# Linux 2.6 kernel module compilation

# -ffast-math -mhard-float build in references to symbols like
# __fixunsdfsi
# __adddf3
# __fixdfsi
# __ledf2
# __divdf3
# __muldf3
# __gedf2
# __subdf3

EXTRA_CFLAGS += -I/usr/include -I/usr/realtime/include -ffast-math -mhard-float -D__IN_RTAI__ -DEXPORT_SYMTAB

obj-m += rtapi_hal_mod.o

rtapi_hal_mod-objs := rtai_rtapi.o rtai_rtapi_hal.o

clean :
	- \rm -f *.o *.ko

else

KERNEL_SOURCE_DIR = /usr/src/linux
EXTRA_CFLAGS += -I/usr/realtime/include -ffast-math -mhard-float -DHAVE_RTAI -D__IN_RTAI__ -DEXPORT_SYMTAB

modules :
	$(MAKE) -C "$(KERNEL_SOURCE_DIR)" SUBDIRS="$(shell pwd)" $@

obj-m += rtapi_hal_mod.o

rtapi_hal_mod-objs := rtai_rtapi.o rtai_rtapi_hal.o

modules_clean : 
	- rm -f *.o *.ko .*.cmd .*.flags *.mod.c Module.symvers

endif

endif
