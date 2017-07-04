QT += core dbus
QT -= gui

TARGET = future_arbitrageur
CONFIG += console C++14
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../common/common_utility.cpp \
    ../common/connection_manager.cpp \
    future_arbitrageur.cpp \
    depth_market.cpp \
    base_strategy.cpp \
    pair_trade.cpp \
    butterfly.cpp

HEADERS += ../config.h \
    ../common/common_utility.h \
    ../common/connection_manager.h \
    future_arbitrageur.h \
    depth_market.h \
    base_strategy.h \
    pair_trade.h \
    butterfly.h

INCLUDEPATH += ../ ../common/
DBUS_INTERFACES += ../interface/sinyee_replayer.xml ../interface/market_watcher.xml ../interface/trade_executer.xml
