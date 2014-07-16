LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := binder
LOCAL_SRC_FILES := libs/libbinder.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := utils
LOCAL_SRC_FILES := libs/libutils.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := cutils
LOCAL_SRC_FILES := libs/libcutils.so
include $(PREBUILT_SHARED_LIBRARY)

LOCAL_MODULE := sandbox
LOCAL_SHARED_LIBRARIES := binder utils cutils log bionic
# FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)
# FILE_LIST += $(wildcard $(LOCAL_PATH)/*.c)
# LOCAL_SRC_FILES = $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_SRC_FILES += sandbox.c \
	binder.c \
	ptraceaux.c \
	intentfw.cpp \
	proc.c \
	# zygote.c

# LOCAL_ARM_MODE = arm

include $(BUILD_STATIC_LIBRARY)

