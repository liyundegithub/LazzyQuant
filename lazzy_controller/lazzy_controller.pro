QT += core gui widgets dbus

TARGET = lazzy_controller
CONFIG += c++14

TEMPLATE = app

LAZZYQUANT_ROOT = $$PWD/..

SOURCES += \
    $$LAZZYQUANT_ROOT/common/dbus_monitor.cpp \
    mainwindow.cpp \
    main.cpp

HEADERS += \
    $$LAZZYQUANT_ROOT/common/dbus_monitor.h \
    mainwindow.h \
    $$LAZZYQUANT_ROOT/config.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += \
    $$LAZZYQUANT_ROOT \
    $$LAZZYQUANT_ROOT/common

include($$LAZZYQUANT_ROOT/trade_executer/trade_executer_dbus.pri)

DBUS_INTERFACES += \
    $$LAZZYQUANT_ROOT/interface/market_watcher.xml \
    trade_executer_dbus \
    $$LAZZYQUANT_ROOT/interface/tick_replayer.xml \
    $$LAZZYQUANT_ROOT/interface/quant_trader.xml
