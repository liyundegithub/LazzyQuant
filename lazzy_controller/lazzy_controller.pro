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

INCLUDEPATH += ../ ../common/

DBUS_INTERFACES += \
    ../interface/market_watcher.xml \
    ../interface/trade_executer.xml \
    ../interface/sinyee_replayer.xml \
    ../interface/quant_trader.xml
