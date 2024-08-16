include $(CLEAR_VARS)
LOCAL_SRC_FILES		:= example/MCU/mcu_spi_test.c 
LOCAL_C_INCLUDES		:= rc663_opt/
LOCAL_LDFLAGS		+= -Llibs/armeabi-v7a/			
#LOCAL_SHARED_LIBRARIES += cardlan_desfire
LOCAL_SHARED_LIBRARIES += cardlan_StandardBus
LOCAL_MODULE		:= mcu_spi_test
LOCAL_MODULE_TAGS	:= optional
LOCAL_CFLAGS += -O2 -w 
LOCAL_CFLAGS += -pie -fPIE 
LOCAL_LDFLAGS += -pie -fPIE
include $(BUILD_EXECUTABLE)


