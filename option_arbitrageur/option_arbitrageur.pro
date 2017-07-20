QT += core gui dbus xml datavisualization
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = option_arbitrageur
CONFIG += console c++11
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../common/market.cpp \
    ../common/common_utility.cpp \
    ../common/multiple_timer.cpp \
    ../common/trading_calendar.cpp \
    option_arbitrageur.cpp \
    option_pricing.cpp \
    option_helper.cpp \
    high_frequency.cpp \
    risk_free.cpp \
    depth_market.cpp \
    base_strategy.cpp \
    option_index.cpp

INCLUDEPATH += ../ ../common/
DBUS_INTERFACES += ../interface/market_watcher.xml ../interface/trade_executer.xml

HEADERS += ../config.h \
    ../common/market.h \
    ../common/common_utility.h \
    ../common/multiple_timer.h \
    ../common/trading_calendar.h \
    option_arbitrageur.h \
    option_pricing.h \
    option_helper.h \
    high_frequency.h \
    risk_free.h \
    depth_market.h \
    base_strategy.h \
    option_index.h
