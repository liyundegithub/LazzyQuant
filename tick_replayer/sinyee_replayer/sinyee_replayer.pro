QT += core dbus
QT -= gui

TARGET = sinyee_replayer
CONFIG += console c++14
CONFIG -= app_bundle

TEMPLATE = app

LAZZYQUANT_ROOT = $$PWD/../..

SOURCES += main.cpp

INCLUDEPATH += $$LAZZYQUANT_ROOT

include(sinyee_replayer.pri)

DBUS_ADAPTORS += $$LAZZYQUANT_ROOT/interface/tick_replayer.xml
