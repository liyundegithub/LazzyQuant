QT += core dbus
QT -= gui

greaterThan(QT_MAJOR_VERSION, 4): QT += concurrent

TARGET = ctp_executer
CONFIG += console
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

DISTFILES +=

unix {
    HEADERS += \
        ../ctp/linux64/ThostFtdcTraderApi.h \
        ../ctp/linux64/ThostFtdcUserApiDataType.h \
        ../ctp/linux64/ThostFtdcUserApiStruct.h

    INCLUDEPATH += ../ctp/linux64/
    LIBS += ../ctp/linux64/thosttraderapi.so
    QMAKE_CXXFLAGS += -std=c++11
}

win32 {
    HEADERS += \
        ../ctp/win32/ThostFtdcTraderApi.h \
        ../ctp/win32/ThostFtdcUserApiDataType.h \
        ../ctp/win32/ThostFtdcUserApiStruct.h

    INCLUDEPATH += ../ctp/win32/
    LIBS += ../ctp/win32/thosttraderapi.lib
}
