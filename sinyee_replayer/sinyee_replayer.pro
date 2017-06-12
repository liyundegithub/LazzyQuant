QT += core dbus
QT -= gui

TARGET = sinyee_replayer
CONFIG += console c++14
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    sinyee_replayer.cpp \
    sinyee_tick.cpp

HEADERS += \
    sinyee_replayer.h \
    sinyee_tick.h

INCLUDEPATH += ../ ../common/
DBUS_ADAPTORS += ../interface/sinyee_replayer.xml
