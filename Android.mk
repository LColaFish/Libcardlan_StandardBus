LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := crypto
LOCAL_SRC_FILES := ssl/lib/libcrypto.a	
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE :=  ssl
LOCAL_SRC_FILES := ssl/lib/libssl.a
include $(PREBUILT_STATIC_LIBRARY)


include $(LOCAL_PATH)/Android_663.mk
include $(LOCAL_PATH)/Android_example.mk

