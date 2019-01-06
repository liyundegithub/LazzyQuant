QT += core dbus
QT -= gui

TARGET = ctp_executer
CONFIG += console c++11
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../common/common_utility.cpp \
    ../common/message_handler.cpp \
    ctp_executer.cpp \
    trade_handler.cpp \
    order.cpp \
    parked_order.cpp \
    ctp_parked_order.cpp \
    parked_order_dbus.cpp

HEADERS += ../config.h \
    ../common/common_utility.h \
    ../common/message_handler.h \
    ctp_executer.h \
    trade_handler.h \
    order.h \
    parked_order.h

INCLUDEPATH += ../ ../common/

trade_executer_adaptor.files = ../interface/trade_executer.xml
trade_executer_adaptor.header_flags = -i parked_order.h
DBUS_ADAPTORS += trade_executer_adaptor

unix:CTP_FOLDER_PREFIX = linux
win32:CTP_FOLDER_PREFIX = win

contains(QT_ARCH, i386) {
    CTP_FOLDER_SUFFIX = 32
} else {
    CTP_FOLDER_SUFFIX = 64
}

HEADERS += \
    ../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/ThostFtdcTraderApi.h \
    ../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/ThostFtdcUserApiDataType.h \
    ../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/ThostFtdcUserApiStruct.h

INCLUDEPATH += ../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/

unix:LIBS += $$PWD/../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/thosttraderapi.so
win32:LIBS += $$PWD/../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/thosttraderapi.lib
