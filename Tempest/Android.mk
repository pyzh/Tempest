LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := Tempest

Tempest_PATH := $(LOCAL_PATH)

LOCAL_C_INCLUDES := $(Tempest_PATH)/include\
                    $(Tempest_PATH)/math\
                    $(Tempest_PATH)/squish\
                    $(Tempest_PATH)/thirdparty\
                    $(Tempest_PATH)/thirdparty/freetype/include

LOCAL_C_INCLUDES += C:/Users/Try/Home/Programming/android/game_ndk/game/jni/freetype/include

LOCAL_EXPORT_C_INCLUDES := $(Tempest_PATH)/include

LOCAL_CFLAGS    := -std=c++0x
LOCAL_CFLAGS    += -D_GLIBCXX_USE_C99_MATH=1 -DFT2_BUILD_LIBRARY=1 -DFT2_BUILD_LIBRARY=1
LOCAL_CPPFLAGS  := -D__STDC_INT64__ \
                   -Dsigset_t="unsigned int"


FREETYPE_SRC_PATH := $(LOCAL_PATH)/thirdparty/freetype/
LOCAL_SRC_FILES := \
  $(FREETYPE_SRC_PATH)src/autofit/autofit.c \
  $(FREETYPE_SRC_PATH)src/base/basepic.c \
  $(FREETYPE_SRC_PATH)src/base/ftapi.c \
  $(FREETYPE_SRC_PATH)src/base/ftbase.c \
  $(FREETYPE_SRC_PATH)src/base/ftbbox.c \
  $(FREETYPE_SRC_PATH)src/base/ftbitmap.c \
  $(FREETYPE_SRC_PATH)src/base/ftdbgmem.c \
  $(FREETYPE_SRC_PATH)src/base/ftdebug.c \
  $(FREETYPE_SRC_PATH)src/base/ftglyph.c \
  $(FREETYPE_SRC_PATH)src/base/ftinit.c \
  $(FREETYPE_SRC_PATH)src/base/ftpic.c \
  $(FREETYPE_SRC_PATH)src/base/ftstroke.c \
  $(FREETYPE_SRC_PATH)src/base/ftsynth.c \
  $(FREETYPE_SRC_PATH)src/base/ftsystem.c \
  $(FREETYPE_SRC_PATH)src/cff/cff.c \
  $(FREETYPE_SRC_PATH)src/pshinter/pshinter.c \
  $(FREETYPE_SRC_PATH)src/psnames/psnames.c \
  $(FREETYPE_SRC_PATH)src/raster/raster.c \
  $(FREETYPE_SRC_PATH)src/sfnt/sfnt.c \
  $(FREETYPE_SRC_PATH)src/smooth/smooth.c \
  $(FREETYPE_SRC_PATH)src/truetype/truetype.c

LOCAL_SRC_FILES += \
  $(subst $(LOCAL_PATH)/,,\
  $(wildcard $(LOCAL_PATH)/core/wrappers/*.cpp) \
  $(wildcard $(LOCAL_PATH)/core/*.cpp) \
  $(wildcard $(LOCAL_PATH)/dataControl/*.cpp) \
  $(wildcard $(LOCAL_PATH)/math/*.cpp) \
  $(wildcard $(LOCAL_PATH)/ogl/*.cpp) \
  $(wildcard $(LOCAL_PATH)/render/*.cpp) \
  $(wildcard $(LOCAL_PATH)/scene/*.cpp) \
  $(wildcard $(LOCAL_PATH)/shading/*.cpp) \
  $(wildcard $(LOCAL_PATH)/squish/*.cpp) \
  $(wildcard $(LOCAL_PATH)/system/*.cpp) \
  $(wildcard $(LOCAL_PATH)/utils/*.cpp) \
  $(wildcard $(LOCAL_PATH)/ui/*.cpp) \
  $(wildcard $(LOCAL_PATH)/2d/*.cpp) \
  $(wildcard $(LOCAL_PATH)/io/*.cpp) \
  $(wildcard $(LOCAL_PATH)/thirdparty/nv_math/*.cpp) \
  $(wildcard $(LOCAL_PATH)/thirdparty/libpng/*.c) \
  $(wildcard $(LOCAL_PATH)/thirdparty/libjpeg/*.c) \
  $(LOCAL_PATH)/thirdparty/ktx/etc_dec.cpp \
  $(wildcard $(LOCAL_PATH)/*.cpp) )

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

LOCAL_STATIC_LIBRARIES := cpufeatures
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM -lGLESv2 -ljnigraphics -lz

include $(BUILD_SHARED_LIBRARY)
$(call import-module,android/cpufeatures)

