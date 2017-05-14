QT += core dbus concurrent
QT -= gui

TARGET = ctp_executer
CONFIG += console c++11
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../common/utility.cpp \
    ../common/multiple_timer.cpp \
    ctp_executer.cpp \
    trade_handler.cpp \
    order.cpp

HEADERS += ../config.h \
    ../common/expires.h \
    ../common/utility.h \
    ../common/multiple_timer.h \
    ctp_executer.h \
    trade_handler.h \
    order.h

INCLUDEPATH += ../ ../common/
DBUS_ADAPTORS += ../interface/trade_executer.xml

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
