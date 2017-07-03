QT += core dbus
QT -= gui

CONFIG += c++14

TARGET = future_arbitrageur
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../common/common_utility.cpp \
    future_arbitrageur.cpp \
    depth_market.cpp \
    base_strategy.cpp \
    pair_trade.cpp \
    butterfly.cpp \
    ../common/connection_manager.cpp

HEADERS += ../config.h \
    ../common/common_utility.h \
    future_arbitrageur.h \
    depth_market.h \
    base_strategy.h \
    pair_trade.h \
    butterfly.h \
    ../common/connection_manager.h

INCLUDEPATH += ../ ../common/
DBUS_INTERFACES += ../interface/sinyee_replayer.xml ../interface/market_watcher.xml ../interface/trade_executer.xml
