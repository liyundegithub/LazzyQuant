#ifndef QUANT_TRADER_MANAGER_H
#define QUANT_TRADER_MANAGER_H

#include "trading_calendar.h"
#include "multiple_timer.h"
#include "db_helper.h"
#include "connection_manager.h"
#include "abstract_manager.h"

#include <QCoreApplication>
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
    delete marketOpenTimer ;
    delete marketCloseTimer;
}

template<class W, class T, class E>
void QuantTraderManagerReal<W, T, E>::init()
{
    QuantTraderManager<W, T, E>::init();

    MultipleTimer *marketOpenTimer = new MultipleTimer({{8, 50}, {20, 50}});
    QObject::connect(marketOpenTimer, &MultipleTimer::timesUp, [=]() -> void {
                         if (this->pSource->getStatus() == "Ready") {
                             QString tradingDay = this->pSource->getTradingDay();
                             this->pSource->setTradingDay(tradingDay);
                             this->pTrader->setTradingDay(tradingDay);
                         } else {
                             qWarning() << "Market Watcher Not Ready!";
                         }
                         checkAndReopenDbIfNotAlive();
                     });

    QList<QTime> timePoints({{2, 35}, {11, 35}, {15, 5}});
    int tradingTimeSegments = timePoints.size();
    MultipleTimer *marketCloseTimer = new MultipleTimer(timePoints);
    QObject::connect(marketCloseTimer, &MultipleTimer::timesUp, [=](int idx) -> void {
        if (idx == (tradingTimeSegments - 1)) {
            this->pTrader->onMarketClose();
        } else {
            this->pTrader->onMarketPause();
        }
    });
}

template<class R, class T, class E>
class QuantTraderManagerReplay : public QuantTraderManager<R, T, E>
{
    QString startDay;
    QString endDay;
    bool autoStartReplay = false;
    bool quitAfterReplay = false;

public:
    QuantTraderManagerReplay(R *pReplayer, T *pTrader, E *pExecuter);
    ~QuantTraderManagerReplay() {}

    void setAutoReplayDate(const QString &startDay, const QString &endDay, bool quitAfterReplay = false);
    void init() override;

};

template<class R, class T, class E>
QuantTraderManagerReplay<R, T, E>::QuantTraderManagerReplay(R *pReplayer, T *pTrader, E *pExecuter) :
    QuantTraderManager<R, T, E>(pReplayer, pTrader, pExecuter)
{
}

template<class R, class T, class E>
void QuantTraderManagerReplay<R, T, E>::setAutoReplayDate(const QString &startDay, const QString &endDay, bool quitAfterReplay)
{
    this->startDay = startDay;
    this->endDay = endDay;
    this->autoStartReplay = true;
    this->quitAfterReplay = quitAfterReplay;
}

template<class R, class T, class E>
void QuantTraderManagerReplay<R, T, E>::init()
{
    QuantTraderManager<R, T, E>::init();

    QObject::connect(this->pSource, SIGNAL(endOfReplay(QString)), this->pTrader, SLOT(onMarketClose()));

    if (autoStartReplay) {
        QStringList replayDates;
        QDate startDate = QDate::fromString(startDay, QStringLiteral("yyyyMMdd"));
        QDate endDate = QDate::fromString(endDay, QStringLiteral("yyyyMMdd"));
        for (QDate date = startDate; date <= endDate; date = date.addDays(1)) {
            if (TradingCalendar::getInstance()->isTradingDay(date)) {
                replayDates << date.toString(QStringLiteral("yyyyMMdd"));
            }
        }

        if (quitAfterReplay) {
            QObject::connect(this->pSource, &R::endOfReplay, [=](const QString &day) -> void {
                if (day == endDay) {
                    QCoreApplication::quit();
                }
            });
        }

        QTimer::singleShot(500, [=]() -> void {
            for (const auto& date : qAsConst(replayDates)) {
                this->pSource->startReplay(date);
            }
        });
    }
}

#endif // QUANT_TRADER_MANAGER_H
