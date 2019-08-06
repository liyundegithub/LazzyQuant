QT += core dbus
QT -= gui

TARGET = ctp_replayer
CONFIG += console c++14
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../../common/common_utility.cpp \
    ../../common/trading_calendar.cpp \
    ../../common/time_mapper.cpp \
    ../tick_replayer.cpp \
    ctp_replayer.cpp

HEADERS += \
    ../../common/common_utility.h \
    ../../common/trading_calendar.h \
    ../../common/time_mapper.h \
    ../common_tick.h \
    ../tick_replayer.h \
    ctp_replayer.h

INCLUDEPATH += ../ ../../ ../../common/
DBUS_ADAPTORS += ../../interface/tick_replayer.xml

unix:CTP_FOLDER_PREFIX = linux
win32:CTP_FOLDER_PREFIX = win

contains(QT_ARCH, i386) {
    CTP_FOLDER_SUFFIX = 32
} else {
    CTP_FOLDER_SUFFIX = 64
}

HEADERS += \
    ../../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/ThostFtdcUserApiDataType.h

INCLUDEPATH += ../../ctp/$$CTP_FOLDER_PREFIX$$CTP_FOLDER_SUFFIX/
