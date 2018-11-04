#ifndef QUANT_TRADER_MANAGER_DBUS_H
#define QUANT_TRADER_MANAGER_DBUS_H

#include "quant_trader_manager.h"


class QuantTraderManagerDbus : public QuantTraderManager
{
    Q_OBJECT
public:
    QuantTraderManagerDbus(bool replayMode = true, ReplaySource replaySource = SINYEE_REPLAYER);
    ~QuantTraderManagerDbus();
};

#endif // QUANT_TRADER_MANAGER_DBUS_H
