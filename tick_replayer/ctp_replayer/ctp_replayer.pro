QT += core dbus
QT -= gui

TARGET = ctp_replayer
CONFIG += console c++14
CONFIG -= app_bundle

TEMPLATE = app

LAZZYQUANT_ROOT = $$PWD/../..

SOURCES += main.cpp

INCLUDEPATH += $$LAZZYQUANT_ROOT

include(ctp_replayer.pri)

DBUS_ADAPTORS += $$LAZZYQUANT_ROOT/interface/tick_replayer.xml
