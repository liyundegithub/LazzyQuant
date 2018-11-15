#ifndef QUANT_TRADER_MANAGER_BUNDLE_H
#define QUANT_TRADER_MANAGER_BUNDLE_H

#include "abstract_manager.h"

#include <QPair>

class MarketWatcher;
class SinYeeReplayer;
class QuantTrader;
class CtpExecuter;

class QuantTraderManagerBundle
{
    AbstractManager *d = nullptr;

public:
    QuantTraderManagerBundle(MarketWatcher *pWatcher,
                             QuantTrader *pTrader,
                             CtpExecuter *pExecuter,
                             bool replayMode,
                             const QPair<QString, QString> &replayRange = {},
                             bool quitAfterReplay = false);

    QuantTraderManagerBundle(SinYeeReplayer *pReplayer,
                             QuantTrader *pTrader,
                             CtpExecuter *pExecuter,
                             const QPair<QString, QString> &replayRange = {},
                             bool quitAfterReplay = false);

    ~QuantTraderManagerBundle();

};

#endif // QUANT_TRADER_MANAGER_BUNDLE_H
