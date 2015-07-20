LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES:= \
    main.cpp \
    sockets.cpp \
    util.cpp \
	framebuffer.cpp

LOCAL_SRC_FILES += \
		./jpeg/jaricom.c \
		./jpeg/jcapimin.c \
		./jpeg/jcapistd.c \
		./jpeg/jcarith.c \
		./jpeg/jccoefct.c \
		./jpeg/jccolor.c \
		./jpeg/jcdctmgr.c \
		./jpeg/jchuff.c \
		./jpeg/jcinit.c \
		./jpeg/jcmainct.c \
		./jpeg/jcmarker.c \
		./jpeg/jcmaster.c \
		./jpeg/jcomapi.c \
		./jpeg/jcparam.c \
		./jpeg/jcprepct.c \
		./jpeg/jcsample.c \
		./jpeg/jctrans.c \
		./jpeg/jdapimin.c \
		./jpeg/jdapistd.c \
		./jpeg/jdarith.c \
		./jpeg/jdatadst.c \
		./jpeg/jdatasrc.c \
		./jpeg/jdcoefct.c \
		./jpeg/jdcolor.c \
		./jpeg/jddctmgr.c \
		./jpeg/jdhuff.c \
		./jpeg/jdinput.c \
		./jpeg/jdmainct.c \
		./jpeg/jdmarker.c \
		./jpeg/jdmaster.c \
		./jpeg/jdmerge.c \
		./jpeg/jdpostct.c \
		./jpeg/jdsample.c \
		./jpeg/jdtrans.c \
		./jpeg/jerror.c \
		./jpeg/jfdctflt.c \
		./jpeg/jfdctfst.c \
		./jpeg/jfdctint.c \
		./jpeg/jidctflt.c \
		./jpeg/jidctfst.c \
		./jpeg/jidctint.c \
		./jpeg/jmemmgr.c \
		./jpeg/jmemnobs.c \
		./jpeg/jquant1.c \
		./jpeg/jquant2.c \
		./jpeg/jutils.c \
		./jpeg/rdbmp.c \
		./jpeg/rdcolmap.c \
		./jpeg/rdgif.c \
		./jpeg/rdppm.c \
		./jpeg/rdrle.c \
		./jpeg/rdswitch.c \
		./jpeg/rdtarga.c \
		./jpeg/transupp.c \
		./jpeg/wrbmp.c \
		./jpeg/wrgif.c \
		./jpeg/wrppm.c \
		./jpeg/wrrle.c \
		./jpeg/wrtarga.c
		
#LOCAL_SHARED_LIBRARIES := \
	libutils \
    libsurfaceflinger_client \
    libgui \
	libcutils 

LOCAL_MODULE:= ruicapsvc

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += \
		-DHAVE_SYS_UIO_H

#LOCAL_CFLAGS += -O3 -fstrict-aliasing
#LOCAL_CFLAGS += -O3 -fstrict-aliasing -fprefetch-loop-arrays
#LOCAL_CFLAGS += -O3 -fstrict-aliasing  -g \
LOCAL_CFLAGS += -Ofast -fstrict-aliasing -g  \
	-DCONFIG_EMBEDDED \
	-DUSE_IND_THREAD \
#	-D_REENTRANT



LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/jpeg

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/ginger2.3.3.framework.include \
	$(LOCAL_PATH)/ginger2.3.3.system.include \
	$(LOCAL_PATH)/ginger2.3.3.hardware/libhardware_legarcy.include \
	$(LOCAL_PATH)/ginger2.3.3.hardware/libhardware.include

LOCAL_LDLIBS := -llog \
	-L$(LOCAL_PATH)/libs \
	-lutils \
    -lsurfaceflinger_client \
    -lgui \
	-lcutils 

#LOCAL_ALLOW_UNDEFINED_SYMBOLS = true

LOCAL_PRELINK_MODULE:=false
#include $(BUILD_EXECUTABLE)
include $(BUILD_SHARED_LIBRARY)
