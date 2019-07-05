QT += core gui widgets dbus

TARGET = lazzy_controller
CONFIG += c++14

TEMPLATE = app

SOURCES += main.cpp \
    ../common/dbus_monitor.cpp \
    mainwindow.cpp

HEADERS += ../config.h \
    ../common/dbus_monitor.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += ../ ../common/ ../trade_executer/

trade_executer_interface.files = ../interface/trade_executer.xml
trade_executer_interface.header_flags = -i parked_order.h

DBUS_INTERFACES += \
    ../interface/market_watcher.xml \
    trade_executer_interface \
    ../interface/tick_replayer.xml \
    ../interface/quant_trader.xml
