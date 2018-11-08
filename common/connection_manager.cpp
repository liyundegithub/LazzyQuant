#include <QObject>

#include "connection_manager.h"

ConnectionManager::ConnectionManager(const QList<QObject *> &inputs, const QList<QObject *> &strategies)
{
    for (auto * input : inputs) {
        if (input != nullptr) {
            senders << input;
            for (auto * strategy : strategies) {
                if (strategy != nullptr) {
                    QObject::connect(input, SIGNAL(newMarketData(QString,qint64,double,int,double,int,double,int)),
                                     strategy, SLOT(onMarketData(QString,qint64,double,int,double,int,double,int)), Qt::UniqueConnection);
                    QObject::connect(input, SIGNAL(tradingDayChanged(QString)),
                                     strategy, SLOT(setTradingDay(QString)), Qt::UniqueConnection);
                }
            }
        }
    }
}

ConnectionManager::~ConnectionManager()
{
    for (auto * sender : qAsConst(senders)) {
        sender->disconnect();
    }
}
