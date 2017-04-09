QT += core dbus xml
QT -= gui

TARGET = option_arbitrageur
CONFIG += console c++11
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    option_arbitrageur.cpp \
    ../common/market.cpp \
    ../common/utility.cpp \
    ../common/multiple_timer.cpp

INCLUDEPATH += ../ ../common/
DBUS_INTERFACES += ../interface/market_watcher.xml ../interface/trade_executer.xml

HEADERS += ../config.h \
    option_arbitrageur.h \
    ../common/market.h \
    ../common/utility.h \
    ../common/multiple_timer.h
