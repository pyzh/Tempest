mac: QMAKE_CFLAGS += -Wno-self-assign

HEADERS += \
    $$PWD/png.h \
    $$PWD/pngconf.h \
    $$PWD/pngpriv.h \
    $$PWD/pnglibconf.h \
    $$PWD/pngstruct.h \
    $$PWD/pnginfo.h

SOURCES += \
    $$PWD/png.c \
    $$PWD/pngerror.c \
    $$PWD/pngget.c \
    $$PWD/pngmem.c \
    $$PWD/pngpread.c \
    $$PWD/pngread.c \
    $$PWD/pngrio.c \
    $$PWD/pngrtran.c \
    $$PWD/pngrutil.c \
    $$PWD/pngset.c \
    $$PWD/pngtrans.c \
    $$PWD/pngwio.c \
    $$PWD/pngwrite.c \
    $$PWD/pngwtran.c \
    $$PWD/pngwutil.c
