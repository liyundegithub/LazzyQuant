QT += core dbus
QT -= gui

TARGET = option_arbitrageur
CONFIG += console c++14
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../common/common_utility.cpp \
    ../common/multiple_timer.cpp \
    ../common/connection_manager.cpp \
    ../common/trading_calendar.cpp \
    ../common/message_handler.cpp \
    option_arbitrageur.cpp \
    option_arbitrageur_dbus.cpp \
    option_pricing.cpp \
    option_helper.cpp \
    high_frequency.cpp \
    risk_free.cpp \
    depth_market.cpp \
    base_strategy.cpp \
    option_index.cpp

HEADERS += ../config.h \
    ../common/common_utility.h \
    ../common/multiple_timer.h \
    ../common/connection_manager.h \
    ../common/trading_calendar.h \
    ../common/message_handler.h \
    option_arbitrageur.h \
    option_arbitrageur_dbus.h \
    option_arbitrageur_manager.h \
    option_arbitrageur_options.h \
    option_pricing.h \
    option_helper.h \
    high_frequency.h \
    risk_free.h \
    depth_market.h \
    base_strategy.h \
    option_index.h

INCLUDEPATH += ../ ../common/ ../trade_executer/

trade_executer_interface.files = ../interface/trade_executer.xml
trade_executer_interface.header_flags = -i parked_order.h

DBUS_INTERFACES += \
    ../interface/market_watcher.xml \
    ../interface/tick_replayer.xml \
    trade_executer_interface
