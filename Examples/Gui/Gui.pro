TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../../Tempest/include
LIBS += -L../../lib/ -lTempest

DESTDIR = ../bin

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h


