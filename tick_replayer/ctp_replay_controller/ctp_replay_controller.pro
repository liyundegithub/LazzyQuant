QT += core gui widgets dbus

TARGET = ctp_replay_controller
CONFIG += c++14

TEMPLATE = app

LAZZYQUANT_ROOT = $$PWD/../..

SOURCES += main.cpp

INCLUDEPATH += $$LAZZYQUANT_ROOT

include(../ctp_replayer/ctp_replayer.pri)
include(../replay_controller.pri)

DBUS_ADAPTORS += $$LAZZYQUANT_ROOT/interface/tick_replayer.xml
