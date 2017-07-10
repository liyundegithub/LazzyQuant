#ifndef STRATEGY_STATUS_H
#define STRATEGY_STATUS_H

#include <QString>
#include <boost/optional.hpp>

class QSettings;

struct StrategyStatus {
    boost::optional<int> position;
    boost::optional<double> takeProfit;
    boost::optional<double> stopLoss;
};

class StrategyStatusManager {
    QSettings *pSettings;

public:
    StrategyStatusManager();
    ~StrategyStatusManager();

    StrategyStatus getStatus(const QString &strategyID);
    void setStatus(const QString &strategyID, const StrategyStatus &status);
    boost::optional<int> getPosition(const QString &strategyID);
    void setPosition(const QString &strategyID, const boost::optional<int> &position);
};

#endif // STRATEGY_STATUS_H
