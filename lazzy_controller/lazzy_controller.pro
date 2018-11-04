QT += core gui widgets dbus

TARGET = lazzy_controller
CONFIG += c++14

TEMPLATE = app

SOURCES += main.cpp \
    ../common/common_utility.cpp \
    ../common/dbus_monitor.cpp \
    mainwindow.cpp \
    quant_trader_manager.cpp \
    quant_trader_manager_dbus.cpp

HEADERS += ../config.h \
    ../common/common_utility.h \
    ../common/dbus_monitor.h \
    mainwindow.h \
    abstract_manager.h \
    quant_trader_manager.h \
    quant_trader_manager_dbus.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += ../ ../common/
DBUS_INTERFACES += \
    ../interface/market_watcher.xml \
    ../interface/trade_executer.xml \
    ../interface/sinyee_replayer.xml \
    ../interface/quant_trader.xml
