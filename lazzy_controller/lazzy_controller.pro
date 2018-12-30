QT += core gui widgets dbus

TARGET = lazzy_controller
CONFIG += c++14

TEMPLATE = app

SOURCES += main.cpp \
    ../common/dbus_monitor.cpp \
    ../trade_executer/parked_order.cpp \
    mainwindow.cpp

HEADERS += ../config.h \
    ../common/dbus_monitor.h \
    ../trade_executer/parked_order.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += ../ ../common/ ../trade_executer/

trade_executer_interface.files = ../interface/trade_executer.xml
trade_executer_interface.header_flags = -i parked_order.h

DBUS_INTERFACES += \
    ../interface/market_watcher.xml \
    trade_executer_interface \
    ../interface/sinyee_replayer.xml \
    ../interface/quant_trader.xml
