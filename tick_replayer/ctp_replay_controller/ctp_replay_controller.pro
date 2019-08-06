QT += core gui widgets dbus

TARGET = ctp_replay_controller
CONFIG += c++14

TEMPLATE = app

SOURCES += main.cpp \
    ../../common/common_utility.cpp \
    ../../common/trading_calendar.cpp \
    ../../common/time_mapper.cpp \
    ../tick_replayer.cpp \
    ../replay_controller.cpp \
    ../control_widget.cpp \
    ../ctp_replayer/ctp_replayer.cpp

HEADERS += \
    ../../common/common_utility.h \
    ../../common/trading_calendar.h \
    ../../common/time_mapper.h \
    ../common_tick.h \
    ../tick_replayer.h \
    ../replay_controller.h \
    ../control_widget.h \
    ../ctp_replayer/ctp_replayer.h

INCLUDEPATH += ../ ../../ ../../common/ ../ctp_replayer/
DBUS_ADAPTORS += ../../interface/tick_replayer.xml

FORMS += \
    ../control_widget.ui

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
