include $(CLEAR_VARS)
LOCAL_SRC_FILES		:= example/Mifare/MIFARE_DES/mifare_des_test.c 
LOCAL_C_INCLUDES	:= rc663_opt/
#LOCAL_C_INCLUDES	:= rc531_opt/
LOCAL_LDFLAGS		+= -Llibs/armeabi-v7a/			
LOCAL_SHARED_LIBRARIES += cardlan_StandardBus663
#LOCAL_SHARED_LIBRARIES += cardlan_StandardBus531
LOCAL_MODULE		:= mifare_des_test
LOCAL_MODULE_TAGS	:= optional
LOCAL_CFLAGS += -O2 -w 
LOCAL_CFLAGS += -pie -fPIE 
LOCAL_LDFLAGS += -pie -fPIE
include $(BUILD_EXECUTABLE)


