#-------------------------------------------------
#
# Project created by QtCreator 2012-08-19T21:22:12
#
#-------------------------------------------------

QT       -= core gui

TARGET = liboac
TEMPLATE = lib

DEFINES += LIBOAC_LIBRARY

QMAKE_INCDIR = include

SOURCES += \
    src/test.cpp \
    src/serial.cpp \
    src/oacsp-utils.cpp \
    src/devices.cpp

HEADERS +=\
    include/types.h \
    include/test.h \
    include/serial.h \
    include/platform.h \
    include/oacsp-utils.h \
    include/oacsp.h \
    include/exception.h \
    include/events.h \
    include/devices.h \
    include/components.h

win32 {
   CONFIG += dll staticlib
}
