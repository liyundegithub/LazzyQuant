#ifndef QUANT_TRADER_MANAGER_H
#define QUANT_TRADER_MANAGER_H

#include "abstract_manager.h"

#include <QObject>

class QuantTraderManager : public QObject, public AbstractManager
{
    Q_OBJECT
public:
    enum ReplaySource {
        MARKET_WATCHER,
        SINYEE_REPLAYER
    };
    Q_ENUM(ReplaySource)

protected:
    QuantTraderManager(bool replayMode = true);
    ~QuantTraderManager();

};

#endif // QUANT_TRADER_MANAGER_H
