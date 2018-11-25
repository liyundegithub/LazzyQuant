QT += core sql xml concurrent
QT -= gui

TARGET = quant_trader_bundle
CONFIG += console c++14
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../../common/market.cpp \
    ../../common/multiple_timer.cpp \
    ../../common/connection_manager.cpp \
    ../../common/trading_calendar.cpp \
    ../../common/time_mapper.cpp \
    ../../common/common_utility.cpp \
    ../../common/message_handler.cpp \
    ../../common/db_helper.cpp \
    ../../common/trade_logger.cpp \
    ../../market_watcher/market_watcher.cpp \
    ../../market_watcher/tick_receiver.cpp \
    ../../sinyee_replayer/sinyee_replayer.cpp \
    ../../sinyee_replayer/sinyee_tick.cpp \
    ../../sinyee_replayer/sinyee_bar.cpp \
    ../../trade_executer/ctp_executer.cpp \
    ../../trade_executer/trade_handler.cpp \
    ../../trade_executer/order.cpp \
    ../bar.cpp \
    ../bar_collector.cpp \
    ../quant_trader.cpp \
    ../editable.cpp \
    ../indicators_and_strategies.cpp \
    ../indicator/abstract_indicator.cpp \
    ../indicator/indicator_functions.cpp \
    ../indicator/mql5_indicator.cpp \
    ../indicator/ma.cpp \
    ../indicator/macd.cpp \
    ../indicator/parabolicsar.cpp \
    ../indicator/bollinger_band.cpp \
    ../indicator/awesome_oscillator.cpp \
    ../indicator/divergent_bar.cpp \
    ../indicator/fractal.cpp \
    ../indicator/zen/segment.cpp \
    ../strategy/template/trailing_stop.cpp \
    ../strategy/template/single_time_frame_strategy.cpp \
    ../strategy/template/multi_time_frame_strategy.cpp \
    ../strategy/DblMaPsar_strategy.cpp \
    ../strategy/BigHit_strategy.cpp \
    ../strategy/addon_trailingstop.cpp \
    ../strategy/chaos2.cpp \
    ../strategy/lemon1.cpp \
    ../quant_trader_manager_bundle.cpp

HEADERS += ../../config.h \
    ../../common/market.h \
    ../../common/multiple_timer.h \
    ../../common/connection_manager.h \
    ../../common/trading_calendar.h \
    ../../common/time_mapper.h \
    ../../common/common_utility.h \
    ../../common/message_handler.h \
    ../../common/db_helper.h \
    ../../common/trade_logger.h \
    ../../common/abstract_manager.h \
    ../../market_watcher/market_watcher.h \
    ../../market_watcher/tick_receiver.h \
    ../../sinyee_replayer/sinyee_replayer.h \
    ../../sinyee_replayer/sinyee_tick.h \
    ../../sinyee_replayer/sinyee_bar.h \
    ../../trade_executer/ctp_executer.h \
    ../../trade_executer/trade_handler.h \
    ../../trade_executer/order.h \
    ../bar.h \
    ../bar_collector.h \
    ../quant_trader.h \
    ../mql5_compatible.h \
    ../mql5_moving_averages.h \
    ../editable.h \
    ../indicators_and_strategies.h \
    ../indicator/abstract_indicator.h \
    ../indicator/indicator_functions.h \
    ../indicator/mql5_indicator.h \
    ../indicator/ma.h \
    ../indicator/macd.h \
    ../indicator/parabolicsar.h \
    ../indicator/bollinger_band.h \
    ../indicator/awesome_oscillator.h \
    ../indicator/divergent_bar.h \
    ../indicator/fractal.h \
    ../indicator/zen/segment.h \
    ../strategy/template/trailing_stop.h \
    ../strategy/template/abstract_strategy.h \
    ../strategy/template/single_time_frame_strategy.h \
    ../strategy/template/multi_time_frame_strategy.h \
    ../strategy/DblMaPsar_strategy.h \
    ../strategy/BigHit_strategy.h \
    ../strategy/addon_trailingstop.h \
    ../strategy/chaos2.h \
    ../strategy/lemon1.h \
    ../quant_trader_manager.h \
    ../quant_trader_manager_bundle.h

INCLUDEPATH += \
    ../ \
    ../../ \
    ../../common/ \
    ../../market_watcher/ \
    ../../sinyee_replayer/ \
    ../../trade_executer/

unix:CTP_FOLDER_PREFIX = linux
win32:CTP_FOLDER_PREFIX = win

contains(QT_ARCH, i386) {
    CTP_FOLDER_SUFFIX = 32
} else {
    CTP_FOLDER_SUFFIX = 64
}

HEADERS += \
    ../../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/ThostFtdcMdApi.h \
    ../../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/ThostFtdcTraderApi.h \
    ../../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/ThostFtdcUserApiDataType.h \
    ../../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/ThostFtdcUserApiStruct.h

INCLUDEPATH += ../../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/

unix:LIBS += \
    $$PWD/../../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/thostmduserapi.so \
    $$PWD/../../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/thosttraderapi.so

win32:LIBS += \
    $$PWD/../../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/thostmduserapi.lib \
    $$PWD/../../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/thosttraderapi.lib
