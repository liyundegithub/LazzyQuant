QT += core dbus xml
QT -= gui

TARGET = quant_trader
CONFIG += console c++11
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../common/market.cpp \
    ../common/common_utility.cpp \
    ../common/multiple_timer.cpp \
    ../common/connection_manager.cpp \
    ../common/strategy_status.cpp \
    ../common/trading_calendar.cpp \
    quant_trader.cpp \
    bar_collector.cpp \
    bar.cpp \
    indicator/abstract_indicator.cpp \
    indicator/indicator_functions.cpp \
    indicator/mql5_indicator.cpp \
    indicator/ma.cpp \
    indicator/parabolicsar.cpp \
    indicator/bollinger_band.cpp \
    indicator/awesome_oscillator.cpp \
    strategy/abstract_strategy.cpp \
    strategy/DblMaPsar_strategy.cpp \
    strategy/BigHit_strategy.cpp

INCLUDEPATH += ../ ../common/
DBUS_INTERFACES += ../interface/market_watcher.xml ../interface/trade_executer.xml ../interface/sinyee_replayer.xml

HEADERS += ../config.h \
    ../common/market.h \
    ../common/common_utility.h \
    ../common/multiple_timer.h \
    ../common/connection_manager.h \
    ../common/strategy_status.h \
    ../common/trading_calendar.h \
    quant_trader.h \
    bar_collector.h \
    bar.h \
    mql5_compatible.h \
    indicator/abstract_indicator.h \
    indicator/indicator_functions.h \
    indicator/mql5_indicator.h \
    indicator/ma.h \
    indicator/parabolicsar.h \
    indicator/bollinger_band.h \
    indicator/awesome_oscillator.h \
    strategy/abstract_strategy.h \
    strategy/DblMaPsar_strategy.h \
    strategy/BigHit_strategy.h
