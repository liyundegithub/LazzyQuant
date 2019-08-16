QT += core dbus
QT -= gui

TARGET = future_arbitrageur
CONFIG += console c++14
CONFIG -= app_bundle

TEMPLATE = app

LAZZYQUANT_ROOT = $$PWD/..

SOURCES += \
    $$LAZZYQUANT_ROOT/common/message_handler.cpp \
    $$LAZZYQUANT_ROOT/common/connection_manager.cpp \
    main.cpp

HEADERS += \
    $$LAZZYQUANT_ROOT/common/message_handler.h \
    $$LAZZYQUANT_ROOT/common/connection_manager.h \
    $$LAZZYQUANT_ROOT/config.h

INCLUDEPATH += \
    $$LAZZYQUANT_ROOT

include(future_arbitrageur.pri)
include($$LAZZYQUANT_ROOT/trade_executer/trade_executer_dbus.pri)

DBUS_INTERFACES += \
    $$LAZZYQUANT_ROOT/interface/tick_replayer.xml \
    $$LAZZYQUANT_ROOT/interface/market_watcher.xml \
    trade_executer_dbus
