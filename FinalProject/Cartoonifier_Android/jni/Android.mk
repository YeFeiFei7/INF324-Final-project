
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

OPENCV_LIB_TYPE:=STATIC
OPENCV_INSTALL_MODULES:=on

include ../includeOpenCV.mk
ifeq ("$(wildcard $(OPENCV_MK_PATH))","")

    include $(TOOLCHAIN_PREBUILT_ROOT)/user/share/OpenCV/OpenCV.mk
else
    include $(OPENCV_MK_PATH)
endif

LOCAL_MODULE    := cartoonifier
LOCAL_LDLIBS +=  -llog -ldl

LOCAL_SRC_FILES := jni_part.cpp
LOCAL_SRC_FILES += ../../Cartoonifier_Desktop/cartoon.cpp
LOCAL_SRC_FILES += ../../Cartoonifier_Desktop/ImageUtils_0.7.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Cartoonifier_Desktop


include $(BUILD_SHARED_LIBRARY)
