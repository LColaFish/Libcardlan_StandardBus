include $(CLEAR_VARS)
LOCAL_SRC_FILES		:= example/nxp/rc531/rc531_test.c 
LOCAL_LDFLAGS		+= -Llibs/armeabi-v7a/			
LOCAL_SHARED_LIBRARIES += rc663nxp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/nxprdlib/include/intfs
LOCAL_C_INCLUDES += $(LOCAL_PATH)/nxprdlib/include/types
LOCAL_MODULE		:= rc531_test
LOCAL_MODULE_TAGS	:= optional
LOCAL_CFLAGS += -O2 -w 
LOCAL_CFLAGS += -pie -fPIE 
LOCAL_LDFLAGS += -pie -fPIE
include $(BUILD_EXECUTABLE)


