#ifndef CONFIG_H
#define CONFIG_H

#include "config_struct.h"

#define PROJECT_NAME                "LazzyQuant"
#define LOWER_CASE_NAME             "lazzyquant"
#define UNIQ_PREFIX                 "com." LOWER_CASE_NAME "."

#define ORGANIZATION                PROJECT_NAME

#define WATCHER_NAME                "ctp_watcher"
#define EXECUTER_NAME               "ctp_executer"

#define WATCHER_DBUS_SERVICE        UNIQ_PREFIX WATCHER_NAME
#define EXECUTER_DBUS_SERVICE       UNIQ_PREFIX EXECUTER_NAME
#define WATCHER_DBUS_OBJECT         "/watcher"
#define EXECUTER_DBUS_OBJECT        "/executer"

const CONFIG_ITEM watcherConfigs[] = {
    {WATCHER_NAME, ORGANIZATION, WATCHER_DBUS_OBJECT, WATCHER_DBUS_SERVICE}
};

const CONFIG_ITEM executerConfigs[] = {
    {EXECUTER_NAME, ORGANIZATION, EXECUTER_DBUS_OBJECT, EXECUTER_DBUS_SERVICE}
};

#endif // CONFIG_H
