QT += core gui widgets dbus

TARGET = lazzy_controller
CONFIG += c++14

TEMPLATE = app

SOURCES += main.cpp \
    ../common/common_utility.cpp \
    ../common/multiple_timer.cpp \
    ../common/connection_manager.cpp \
    ../common/trading_calendar.cpp \
    ../common/dbus_monitor.cpp \
    mainwindow.cpp

HEADERS += ../config.h \
    ../common/common_utility.h \
    ../common/multiple_timer.h \
    ../common/connection_manager.h \
    ../common/trading_calendar.h \
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
