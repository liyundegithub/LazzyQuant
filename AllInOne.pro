QT += core xml concurrent
QT -= gui

TARGET = all_in_one
CONFIG += console c++11
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    common/market.cpp \
    common/utility.cpp \
    common/multiple_timer.cpp \
    common/trading_calendar.cpp \
    market_watcher/market_watcher.cpp \
    market_watcher/tick_receiver.cpp \
    trade_executer/ctp_executer.cpp \
    trade_executer/trade_handler.cpp \
    trade_executer/order.cpp \
    option_arbitrageur/option_arbitrageur.cpp \
    option_arbitrageur/option_pricing.cpp \
    option_arbitrageur/option_helper.cpp \
    option_arbitrageur/high_frequency.cpp \
    option_arbitrageur/risk_free.cpp \
    option_arbitrageur/depth_market.cpp \
    option_arbitrageur/base_strategy.cpp \
    option_arbitrageur/option_index.cpp

INCLUDEPATH += common/ market_watcher/ trade_executer/ option_arbitrageur/

HEADERS += config.h \
    common/market.h \
    common/utility.h \
    common/multiple_timer.h \
    common/trading_calendar.h \
    market_watcher/market_watcher.h \
    market_watcher/tick_receiver.h \
    trade_executer/ctp_executer.h \
    trade_executer/trade_handler.h \
    trade_executer/order.h \
    option_arbitrageur/option_arbitrageur.h \
    option_arbitrageur/option_pricing.h \
    option_arbitrageur/option_helper.h \
    option_arbitrageur/high_frequency.h \
    option_arbitrageur/risk_free.h \
    option_arbitrageur/depth_market.h \
    option_arbitrageur/base_strategy.h \
    option_arbitrageur/option_index.h

unix:CTP_FOLDER_PREFIX = linux
win32:CTP_FOLDER_PREFIX = win

contains(QT_ARCH, i386) {
    CTP_FOLDER_SUFFIX = 32
} else {
    CTP_FOLDER_SUFFIX = 64
}

HEADERS += \
    ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/ThostFtdcMdApi.h \
    ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/ThostFtdcUserApiDataType.h \
    ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/ThostFtdcUserApiStruct.h

INCLUDEPATH += ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/

unix:LIBS += $$PWD/ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/thostmduserapi.so $$PWD/ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/thosttraderapi.so
win32:LIBS += $$PWD/ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/thostmduserapi.lib $$PWD/ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/thosttraderapi.lib
