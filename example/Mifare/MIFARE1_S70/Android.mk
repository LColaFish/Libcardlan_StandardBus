include $(CLEAR_VARS)
LOCAL_SRC_FILES		:= example/Mifare/MIFARE1_S70/mifare_test.c 							
LOCAL_LDFLAGS		+= -Llibs/armeabi-v7a/		
LOCAL_C_INCLUDES	:= rc663_opt/
#LOCAL_SHARED_LIBRARIES += cardlan_desfire
LOCAL_SHARED_LIBRARIES += cardlan_StandardBus
LOCAL_MODULE		:= mifare_test
LOCAL_MODULE_TAGS	:= optional
LOCAL_CFLAGS += -O2 -w 
LOCAL_CFLAGS += -pie -fPIE 
LOCAL_LDFLAGS += -pie -fPIE
include $(BUILD_EXECUTABLE)


