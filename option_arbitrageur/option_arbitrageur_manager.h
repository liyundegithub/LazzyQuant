#ifndef OPTION_ARBITRAGEUR_MANAGER_H
#define OPTION_ARBITRAGEUR_MANAGER_H

#include "abstract_manager.h"

template<class W, class T, class E>
class OptionArbitrageurRealManager : public RealManager<W, T, E>
{
protected:
    bool updateOnly;

public:
    OptionArbitrageurRealManager(W *pWatcher, T *pTrader, E *pExecuter, bool updateOnly);
    ~OptionArbitrageurRealManager() override = default;

    void init() override;
    void prepareOpen() override;
};

template<class W, class T, class E>
OptionArbitrageurRealManager<W, T, E>::OptionArbitrageurRealManager(W *pWatcher, T *pTrader, E *pExecuter, bool updateOnly) :
    RealManager<W, T, E>(pWatcher, pTrader, pExecuter),
    updateOnly(updateOnly)
{
}

template<class W, class T, class E>
void OptionArbitrageurRealManager<W, T, E>::init()
{
    if (!updateOnly) {
        AbstractManager::makeDefaultConnections();
    }
    RealManager<W, T, E>::init();
}

template<class W, class T, class E>
void OptionArbitrageurRealManager<W, T, E>::prepareOpen()
{
    if (updateOnly) {
        const QStringList subscribedInstruments = this->pWatcher->getSubscribeList();
        const QStringList cachedInstruments = this->pExecuter->getCachedInstruments();
        const QSet<QString> underlyingIDs = this->pTrader->getUnderlyingIDs();
        QStringList instrumentsToSubscribe;
        for (const auto &item : cachedInstruments) {
            if (!subscribedInstruments.contains(item)) {
                for (const auto &underlyingID : qAsConst(underlyingIDs)) {
                    if (item.startsWith(underlyingID)) {
                        instrumentsToSubscribe << item;
                        break;
                    }
                }
            }
        }
        if (!instrumentsToSubscribe.empty()) {
            this->pWatcher->subscribeInstruments(instrumentsToSubscribe);
        }
    }
}

#endif // OPTION_ARBITRAGEUR_MANAGER_H
