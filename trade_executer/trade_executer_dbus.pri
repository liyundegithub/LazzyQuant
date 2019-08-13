TRADE_EXECUTER_FOLDER = $$LAZZYQUANT_ROOT/trade_executer

SOURCES *= $$TRADE_EXECUTER_FOLDER/parked_order_dbus.cpp

INCLUDEPATH *= $$TRADE_EXECUTER_FOLDER

trade_executer_dbus.files = $$LAZZYQUANT_ROOT/interface/trade_executer.xml
trade_executer_dbus.header_flags = -i parked_order.h
