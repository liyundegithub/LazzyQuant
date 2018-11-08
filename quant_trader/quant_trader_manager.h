#ifndef QUANT_TRADER_MANAGER_H
#define QUANT_TRADER_MANAGER_H

#include "trading_calendar.h"
#include "multiple_timer.h"
#include "connection_manager.h"
#include "abstract_manager.h"

#include <QTimer>
#include <QDebug>

template<class S, class T, class E>
class QuantTraderManager : public AbstractManager
{
public:
    S *pSource;
    T *pTrader;
    E *pExecuter;
    ConnectionManager *pConnManager = nullptr;

    QuantTraderManager(S *pSource, T *pTrader, E *pExecuter);
    ~QuantTraderManager();

    QObject *getSource() const { return pSource; }
    QObject *getTrader() const { return pTrader; }
    QObject *getExecuter() const { return pExecuter; }
    void init() override;

};

template<class S, class T, class E>
QuantTraderManager<S, T, E>::QuantTraderManager(S *pSource, T *pTrader, E *pExecuter) :
    AbstractManager(),
    pSource(pSource),
    pTrader(pTrader),
    pExecuter(pExecuter)
{
}

template<class S, class T, class E>
QuantTraderManager<S, T, E>::~QuantTraderManager()
{
    if (pConnManager)
        delete pConnManager;
}

template<class S, class T, class E>
void QuantTraderManager<S, T, E>::init()
{
    pConnManager = new ConnectionManager({pSource}, {pTrader});
}

template<class W, class T, class E>
class QuantTraderManagerReal : public QuantTraderManager<W, T, E>
{
    MultipleTimer *marketOpenTimer = nullptr;
    MultipleTimer *marketCloseTimer = nullptr;

public:
    QuantTraderManagerReal(W *pWatcher, T *pTrader, E *pExecuter);
    ~QuantTraderManagerReal();

    void init() override;

};

template<class W, class T, class E>
QuantTraderManagerReal<W, T, E>::QuantTraderManagerReal(W *pWatcher, T *pTrader, E *pExecuter) :
    QuantTraderManager<W, T, E>(pWatcher, pTrader, pExecuter)
{
}

template<class W, class T, class E>
QuantTraderManagerReal<W, T, E>::~QuantTraderManagerReal()
{
    if (marketOpenTimer )
        delete marketOpenTimer ;
    if (marketCloseTimer)
        delete marketCloseTimer;
}

template<class W, class T, class E>
void QuantTraderManagerReal<W, T, E>::init()
{
    QuantTraderManager<W, T, E>::init();

    MultipleTimer *marketOpenTimer = new MultipleTimer({{8, 50}, {20, 50}});
    QObject::connect(marketOpenTimer, &MultipleTimer::timesUp, [=]() -> void {
                         if (pSource->getStatus() == "Ready") {
                             QString tradingDay = pSource->getTradingDay();
                             pSource->setTradingDay(tradingDay);
                             pTrader->setTradingDay(tradingDay);
                         } else {
                             qWarning() << "Market Watcher Not Ready!";
                         }
                         pTrader->checkDataBaseStatus();
                     });

    QList<QTime> timePoints({{2, 35}, {11, 35}, {15, 5}});
    int tradingTimeSegments = timePoints.size();
    MultipleTimer *marketCloseTimer = new MultipleTimer(timePoints);
    QObject::connect(marketCloseTimer, &MultipleTimer::timesUp, [=](int idx) -> void {
        if (idx == (tradingTimeSegments - 1)) {
            pTrader->onMarketClose();
        } else {
            pTrader->onMarketPause();
        }
    });
}

template<class R, class T, class E>
class QuantTraderManagerReplay : public QuantTraderManager<R, T, E>
{
    bool autoStartReplay = false;
    QString startDay;
    QString endDay;

public:
    QuantTraderManagerReplay(R *pReplayer, T *pTrader, E *pExecuter);
    ~QuantTraderManagerReplay() {}

    void setAutoReplayDate(const QString &startDay, const QString &endDay);
    void init() override;

};

template<class R, class T, class E>
QuantTraderManagerReplay<R, T, E>::QuantTraderManagerReplay(R *pReplayer, T *pTrader, E *pExecuter) :
    QuantTraderManager<R, T, E>(pReplayer, pTrader, pExecuter)
{
}

template<class R, class T, class E>
void QuantTraderManagerReplay<R, T, E>::setAutoReplayDate(const QString &startDay, const QString &endDay)
{
    this->autoStartReplay = true;
    this->startDay = startDay;
    this->endDay = endDay;
}

template<class R, class T, class E>
void QuantTraderManagerReplay<R, T, E>::init()
{
    QuantTraderManager<R, T, E>::init();

    QObject::connect(pSource, SIGNAL(endOfReplay(QString)), pTrader, SLOT(onMarketClose()));

    if (autoStartReplay) {
        TradingCalendar tc;
        QStringList replayDates;
        QDate startDate = QDate::fromString(startDay, QStringLiteral("yyyyMMdd"));
        QDate endDate = QDate::fromString(endDay, QStringLiteral("yyyyMMdd"));
        for (QDate date = startDate; date <= endDate; date = date.addDays(1)) {
            if (tc.isTradingDay(date)) {
                replayDates << date.toString(QStringLiteral("yyyyMMdd"));
            }
        }

        QTimer::singleShot(500, [=]() -> void {
            for (const auto& date : qAsConst(replayDates)) {
                pSource->startReplay(date);
            }
        });
    }
}

#endif // QUANT_TRADER_MANAGER_H
