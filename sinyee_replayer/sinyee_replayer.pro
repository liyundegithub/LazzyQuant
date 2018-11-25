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
    sinyee_replayer.cpp \
    sinyee_tick.cpp \
    sinyee_bar.cpp

HEADERS += \
    ../common/common_utility.h \
    ../common/trading_calendar.h \
    ../common/time_mapper.h \
    sinyee_replayer.h \
    sinyee_tick.h \
    sinyee_bar.h

INCLUDEPATH += ../ ../common/
DBUS_ADAPTORS += ../interface/sinyee_replayer.xml
