COMMON_FOLDER = $$LAZZYQUANT_ROOT/common

SOURCES *= \
    $$COMMON_FOLDER/message_handler.cpp \
    $$COMMON_FOLDER/connection_manager.cpp

HEADERS *= \
    $$COMMON_FOLDER/message_handler.h \
    $$COMMON_FOLDER/connection_manager.h \
    $$COMMON_FOLDER/abstract_manager.h

INCLUDEPATH *= \
    $$COMMON_FOLDER
