LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    main.cpp \
    sockets.cpp \
    util.cpp \
	framebuffer.cpp \
	ftp.cpp

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
		
LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
    libgui \
    libsurfaceflinger_client \
#	libgui.jelly \
#	libbinder \
#    libui \
#    libandroid_runtime \
#	libz \
#    libnativehelper

ifeq ($(TARGET_AML), 1)
$(info TARGET_AML)
LOCAL_MODULE:= amlscm
LOCAL_CFLAGS +=-DTARGET_AML
else
$(info TARGET_RUI)
LOCAL_MODULE:= ruicapsvc
endif

LOCAL_MODULE_TAGS := optional



LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/jpeg

LOCAL_C_INCLUDES += \


include $(BUILD_EXECUTABLE)
