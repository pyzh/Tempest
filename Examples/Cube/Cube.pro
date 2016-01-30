TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../../Tempest/include
LIBS        += -L../../lib/ -lTempest
LIBS        += -l"gdi32" -l"user32" -l"opengl32"

DESTDIR = ../bin

CONFIG += c++11

win32:{
  #msvc static build
  LIBS += -L"$$(DXSDK_DIR)Lib/x86"
  LIBS += -luser32 -lgdi32 -ld3d11 -ld3dx11 -ld3dcompiler -lopengl32
  }

SOURCES += main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

OTHER_FILES += \
    ../bin/shader/basic.vs.glsl \
    ../bin/shader/basic.fs.glsl \
    ../bin/shader/basic.vs.gl4 \
    ../bin/shader/basic.fs.gl4

