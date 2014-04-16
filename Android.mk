LOCAL_PATH:=$(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	parse_dmi.c \
	dmi_intel.c \

LOCAL_MODULE := libdmi
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := liblog

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT)/sbin
LOCAL_MODULE := intel_fw_props
LOCAL_SRC_FILES:= \
	intel_fw_props.c \

LOCAL_STATIC_LIBRARIES := liblog libdmi libcutils

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_COPY_HEADERS := libdmi.h
LOCAL_COPY_HEADERS_TO := libdmi
include $(BUILD_COPY_HEADERS)
