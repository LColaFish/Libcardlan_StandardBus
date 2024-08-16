LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES		:= libcardlan_StandardBus.c									\
					   libcardlan_mifareplus.c									\
					   libcardlan_StandardBus_util.c	 						\
					   cardlan_devctrl.c										\
					   encrypt_lib/mac.c										\
					   encrypt_lib/des.c										\
					   encrypt_lib/stades.c										\
					   rc663_opt/ISO15693.c										\
					   rc663_opt/ISO15693_operation.c							\
					   rc663_opt/ISO14443.c										\
					   rc663_opt/ISO14443_operation.c							\
					   rc663_opt/pcd663.c										\
					   rc663_opt/nxp_rc663_op.c									\
					   rc663_opt/typea.c										\
					   rc663_opt/libcardlan_iso15693.c							\
    				   mcu_opt/mcu_opt.c										\
					   mcu_opt/psam_opt.c										\
					   card_opt/apdu_cmd.c									     \
						rc663_opt/cpu_card_operation.c							\
						card_opt/M1/M1_Des_card_operation.c						\
						card_opt/M1/M1_Plus_card_operation.c				    \
						card_opt/M1/M1_card_operation.c					        \
						
LOCAL_C_INCLUDES	:= ssl/include/  \
						rc663_opt/
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/nxprdlib/include/intfs
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/nxprdlib/include/types


LOCAL_LDLIBS		+= -lm -llog -ldl -lz 
LOCAL_LDFLAGS		+= -Lssl/lib/ 
LOCAL_STATIC_LIBRARIES += crypto ssl 
	
LOCAL_MODULE		:= libcardlan_Colafish_Psam
LOCAL_MODULE_TAGS	:= optional

LOCAL_CFLAGS += -O2 -w 
include $(BUILD_SHARED_LIBRARY)
