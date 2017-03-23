QT += core dbus xml
QT -= gui

TARGET = quant_trader
CONFIG += console c++11
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    quant_trader.cpp \
    bar_collector.cpp \
    bar.cpp \
    indicator/abstract_indicator.cpp \
    indicator/ma.cpp \
    indicator/parabolicsar.cpp \
    indicator/mql5_indicator.cpp \
    strategy/abstract_strategy.cpp \
    strategy/DblMaPsar_strategy.cpp \
    ../common/market.cpp \
    ../common/utility.cpp

INCLUDEPATH += ../ ../common/
DBUS_INTERFACES += ../interface/market_watcher.xml ../interface/trade_executer.xml

HEADERS += ../config.h \
    quant_trader.h \
    bar_collector.h \
    bar.h \
    mql5_compatible.h \
    indicator/abstract_indicator.h \
    indicator/ma.h \
    indicator/parabolicsar.h \
    indicator/mql5_indicator.h \
    strategy/abstract_strategy.h \
    strategy/DblMaPsar_strategy.h \
    ../common/market.h \
    ../common/utility.h
