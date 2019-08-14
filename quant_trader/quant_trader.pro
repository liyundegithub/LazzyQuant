QT += core dbus sql
QT -= gui

TARGET = quant_trader
CONFIG += console c++14
CONFIG -= app_bundle

TEMPLATE = app

LAZZYQUANT_ROOT = $$PWD/..

SOURCES += \
    quant_trader_dbus.cpp \
    main.cpp

HEADERS += \
    quant_trader_dbus.h \
    quant_trader_options.h \
    quant_trader_manager.h \
    $$LAZZYQUANT_ROOT/config.h

INCLUDEPATH += $$LAZZYQUANT_ROOT

include(quant_trader.pri)
include($$LAZZYQUANT_ROOT/common/common.pri)
include($$LAZZYQUANT_ROOT/trade_executer/trade_executer_dbus.pri)

DBUS_INTERFACES += \
    $$LAZZYQUANT_ROOT/interface/market_watcher.xml \
    $$LAZZYQUANT_ROOT/interface/tick_replayer.xml \
    trade_executer_dbus

DBUS_ADAPTORS += $$LAZZYQUANT_ROOT/interface/quant_trader.xml
