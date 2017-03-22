QT += core dbus xml
QT -= gui

TARGET = market_watcher
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../common/market.cpp \
    ../common/utility.cpp \
    market_watcher.cpp \
    tick_receiver.cpp

HEADERS += ../config.h \
    ../common/market.h \
    ../common/utility.h \
    market_watcher.h \
    tick_receiver.h

INCLUDEPATH += ../ ../common/
DBUS_ADAPTORS += ../interface/market_watcher.xml

DISTFILES +=

unix {
    CTP_FOLDER_PREFIX = linux
    QMAKE_CXXFLAGS += -std=c++11
}

win32:CTP_FOLDER_PREFIX = win

contains(QT_ARCH, i386) {
    CTP_FOLDER_SUFFIX = 32
}else {
    CTP_FOLDER_SUFFIX = 64
}

HEADERS += \
    ../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/ThostFtdcMdApi.h \
    ../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/ThostFtdcUserApiDataType.h \
    ../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/ThostFtdcUserApiStruct.h

INCLUDEPATH += ../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/

unix:LIBS += ../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/thostmduserapi.so
win32:LIBS += ../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/thostmduserapi.lib
