QT += core gui widgets dbus

TARGET = sinyee_replay_controller
CONFIG += c++14

TEMPLATE = app

LAZZYQUANT_ROOT = $$PWD/../..

SOURCES += main.cpp

INCLUDEPATH += $$LAZZYQUANT_ROOT

include(../sinyee_replayer/sinyee_replayer.pri)
include(../replay_controller.pri)

DBUS_ADAPTORS += $$LAZZYQUANT_ROOT/interface/tick_replayer.xml
