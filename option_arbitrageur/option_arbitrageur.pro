QT += core dbus
QT -= gui

TARGET = option_arbitrageur
CONFIG += console c++14
CONFIG -= app_bundle

TEMPLATE = app

LAZZYQUANT_ROOT = $$PWD/..

SOURCES += \
    option_arbitrageur_dbus.cpp \
    main.cpp

HEADERS += \
    option_arbitrageur_dbus.h \
    option_arbitrageur_options.h \
    option_arbitrageur_manager.h \
    $$LAZZYQUANT_ROOT/config.h

INCLUDEPATH += $$LAZZYQUANT_ROOT

include(option_arbitrageur.pri)
include($$LAZZYQUANT_ROOT/common/common.pri)
include($$LAZZYQUANT_ROOT/trade_executer/trade_executer_dbus.pri)

DBUS_INTERFACES += \
    $$LAZZYQUANT_ROOT/interface/market_watcher.xml \
    $$LAZZYQUANT_ROOT/interface/tick_replayer.xml \
    trade_executer_dbus
