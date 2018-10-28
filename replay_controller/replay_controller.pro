QT += core gui widgets dbus

TARGET = replay_controller
CONFIG += c++14

TEMPLATE = app

SOURCES += main2.cpp \
    ../common/common_utility.cpp \
    ../common/trading_calendar.cpp \
    ../common/time_mapper.cpp \
    ../sinyee_replayer/sinyee_replayer.cpp \
    ../sinyee_replayer/sinyee_tick.cpp \
    widget.cpp

HEADERS += \
    ../common/common_utility.h \
    ../common/trading_calendar.h \
    ../common/time_mapper.h \
    ../sinyee_replayer/sinyee_replayer.h \
    ../sinyee_replayer/sinyee_tick.h \
    widget.h

INCLUDEPATH += ../ ../common/ ../sinyee_replayer/
DBUS_ADAPTORS += ../interface/sinyee_replayer.xml

FORMS += \
    widget.ui
