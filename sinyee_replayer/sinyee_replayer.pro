QT += core dbus
QT -= gui

CONFIG += c++14

TARGET = sinyee_replayer
CONFIG += console
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
