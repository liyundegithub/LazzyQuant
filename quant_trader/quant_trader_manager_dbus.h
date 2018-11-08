#ifndef QUANT_TRADER_MANAGER_DBUS_H
#define QUANT_TRADER_MANAGER_DBUS_H

#include "abstract_manager.h"

#include <QObject>
#include <QPair>

class QuantTrader;

class QuantTraderManagerDbus
{
    Q_GADGET

    AbstractManager *d = nullptr;

public:
    enum MarketSource {
        AUTO_SELECT,
        MARKET_WATCHER,
        SINYEE_REPLAYER
    };
    Q_ENUM(MarketSource)
    MarketSource marketSource = AUTO_SELECT;

    QuantTraderManagerDbus(QuantTrader *pTrader,
                           bool replayMode,
                           const QPair<QString, QString> &replayRange = {},
                           MarketSource source = AUTO_SELECT);
    ~QuantTraderManagerDbus();

};

#endif // QUANT_TRADER_MANAGER_DBUS_H
