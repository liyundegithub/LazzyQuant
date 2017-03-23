QT += core dbus
QT -= gui

greaterThan(QT_MAJOR_VERSION, 4): QT += concurrent

TARGET = ctp_executer
CONFIG += console c++11
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ctp_executer.cpp \
    trade_handler.cpp \
    order.cpp

HEADERS += ../config.h \
    ctp_executer.h \
    trade_handler.h \
    order.h \
    ../common/expires.h

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
