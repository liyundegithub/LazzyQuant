QT += core dbus sql
QT -= gui

TARGET = quant_trader
CONFIG += console c++14
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../common/common_utility.cpp \
    ../common/multiple_timer.cpp \
    ../common/connection_manager.cpp \
    ../common/trading_calendar.cpp \
    ../common/message_handler.cpp \
    ../common/db_helper.cpp \
    ../common/trade_logger.cpp \
    quant_trader_manager_dbus.cpp \
    bar.cpp \
    bar_collector.cpp \
    quant_trader.cpp \
    editable.cpp \
    indicators_and_strategies.cpp \
    indicator/abstract_indicator.cpp \
    indicator/indicator_functions.cpp \
    indicator/mql5_indicator.cpp \
    indicator/ma.cpp \
    indicator/macd.cpp \
    indicator/parabolicsar.cpp \
    indicator/bollinger_band.cpp \
    indicator/awesome_oscillator.cpp \
    indicator/divergent_bar.cpp \
    indicator/fractal.cpp \
    indicator/zen/segment.cpp \
    strategy/template/trailing_stop.cpp \
    strategy/template/single_time_frame_strategy.cpp \
    strategy/template/multi_time_frame_strategy.cpp \
    strategy/DblMaPsar_strategy.cpp \
    strategy/BigHit_strategy.cpp \
    strategy/addon_trailingstop.cpp \
    strategy/chaos2.cpp \
    strategy/lemon1.cpp

INCLUDEPATH += ../ ../common/
DBUS_INTERFACES += ../interface/market_watcher.xml ../interface/trade_executer.xml ../interface/sinyee_replayer.xml
DBUS_ADAPTORS += ../interface/quant_trader.xml

HEADERS += ../config.h \
    ../common/common_utility.h \
    ../common/multiple_timer.h \
    ../common/connection_manager.h \
    ../common/trading_calendar.h \
    ../common/message_handler.h \
    ../common/db_helper.h \
    ../common/trade_logger.h \
    ../common/abstract_manager.h \
    quant_trader_manager.h \
    quant_trader_manager_dbus.h \
    bar.h \
    bar_collector.h \
    quant_trader.h \
    mql5_compatible.h \
    mql5_moving_averages.h \
    editable.h \
    indicators_and_strategies.h \
    indicator/abstract_indicator.h \
    indicator/indicator_functions.h \
    indicator/mql5_indicator.h \
    indicator/ma.h \
    indicator/macd.h \
    indicator/parabolicsar.h \
    indicator/bollinger_band.h \
    indicator/awesome_oscillator.h \
    indicator/divergent_bar.h \
    indicator/fractal.h \
    indicator/zen/segment.h \
    strategy/template/trailing_stop.h \
    strategy/template/abstract_strategy.h \
    strategy/template/single_time_frame_strategy.h \
    strategy/template/multi_time_frame_strategy.h \
    strategy/DblMaPsar_strategy.h \
    strategy/BigHit_strategy.h \
    strategy/addon_trailingstop.h \
    strategy/chaos2.h \
    strategy/lemon1.h
