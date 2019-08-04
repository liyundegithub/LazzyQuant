QT += core gui widgets dbus

TARGET = sinyee_replay_controller
CONFIG += c++14

TEMPLATE = app

SOURCES += main.cpp \
    ../../common/common_utility.cpp \
    ../../common/trading_calendar.cpp \
    ../../common/time_mapper.cpp \
    ../common_replayer.cpp \
    ../replay_controller.cpp \
    ../control_widget.cpp \
    ../sinyee_replayer/sinyee_tick.cpp \
    ../sinyee_replayer/sinyee_bar.cpp \
    ../sinyee_replayer/sinyee_replayer.cpp

HEADERS += \
    ../../common/common_utility.h \
    ../../common/trading_calendar.h \
    ../../common/time_mapper.h \
    ../common_tick.h \
    ../common_replayer.h \
    ../replay_controller.h \
    ../control_widget.h \
    ../sinyee_replayer/sinyee_tick.h \
    ../sinyee_replayer/sinyee_bar.h \
    ../sinyee_replayer/sinyee_replayer.h

INCLUDEPATH += ../ ../../ ../../common/ ../sinyee_replayer/
DBUS_ADAPTORS += ../../interface/tick_replayer.xml

FORMS += \
    ../control_widget.ui
