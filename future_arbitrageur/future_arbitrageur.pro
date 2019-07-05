QT += core dbus
QT -= gui

TARGET = future_arbitrageur
CONFIG += console c++14
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../common/common_utility.cpp \
    ../common/connection_manager.cpp \
    ../common/message_handler.cpp \
    ../common/strategy_status.cpp \
    future_arbitrageur.cpp \
    depth_market.cpp \
    base_strategy.cpp \
    pair_trade.cpp \
    butterfly.cpp

HEADERS += ../config.h \
    ../common/common_utility.h \
    ../common/connection_manager.h \
    ../common/message_handler.h \
    ../common/strategy_status.h \
    future_arbitrageur.h \
    depth_market.h \
    base_strategy.h \
    pair_trade.h \
    butterfly.h

INCLUDEPATH += ../ ../common/ ../trade_executer/

trade_executer_interface.files = ../interface/trade_executer.xml
trade_executer_interface.header_flags = -i parked_order.h

DBUS_INTERFACES += ../interface/tick_replayer.xml ../interface/market_watcher.xml trade_executer_interface
