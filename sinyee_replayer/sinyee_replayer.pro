QT += core dbus
QT -= gui

TARGET = sinyee_replayer
CONFIG += console c++14
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../common/common_utility.cpp \
    ../common/trading_calendar.cpp \
    ../common/time_mapper.cpp \
    common_replayer.cpp \
    sinyee_tick.cpp \
    sinyee_bar.cpp \
    sinyee_replayer.cpp

HEADERS += \
    ../common/common_utility.h \
    ../common/trading_calendar.h \
    ../common/time_mapper.h \
    common_tick.h \
    common_replayer.h \
    sinyee_tick.h \
    sinyee_bar.h \
    sinyee_replayer.h

INCLUDEPATH += ../ ../common/
DBUS_ADAPTORS += ../interface/tick_replayer.xml
