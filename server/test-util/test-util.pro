#-------------------------------------------------
#
# Project created by QtCreator 2012-08-19T21:26:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test-util
TEMPLATE = app

QMAKE_INCDIR = include

SOURCES += \
    src/ui-main.cpp \
    src/ctrl-main.cpp \
    src/main.cpp

HEADERS  += \
    include/ui-main.h \
    include/ctrl-main.h

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../liboac/release/ -lliboac
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../liboac/debug/ -lliboac
else:mac: LIBS += -F$$OUT_PWD/../liboac/ -framework liboac
else:symbian: LIBS += -lliboac
else:unix: LIBS += -L$$OUT_PWD/../liboac/ -lliboac

INCLUDEPATH += $$PWD/../liboac/include
DEPENDPATH += $$PWD/../liboac/include

win32 {
   LIBS += -lUser32
}
