#include "connection_manager.h"

ConnectionManager::ConnectionManager(const QObjectList &inputs, const QObjectList &strategies)
{
    for (auto * input : inputs) {
        if (input != nullptr) {
            for (auto * strategy : strategies) {
                if (strategy != nullptr) {
                    auto conn = QObject::connect(input, SIGNAL(newMarketData(QString,qint64,double,int,double,int,double,int)),
                                                strategy, SLOT(onMarketData(QString,qint64,double,int,double,int,double,int)), Qt::UniqueConnection);
                    if (conn) {
                        connections << conn;
                    }
                    conn = QObject::connect(input, SIGNAL(tradingDayChanged(QString)),
                                           strategy, SLOT(setTradingDay(QString)), Qt::UniqueConnection);
                    if (conn) {
                        connections << conn;
                    }
                }
            }
        }
    }
}

ConnectionManager::~ConnectionManager()
{
    for (const auto &connection : qAsConst(connections)) {
        QObject::disconnect(connection);
    }
}
