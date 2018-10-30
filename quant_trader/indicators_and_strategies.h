#ifndef INDICATORS_AND_STRATEGIES_H
#define INDICATORS_AND_STRATEGIES_H

struct QMetaObject;
class QString;

const QMetaObject* getIndicatorMetaObject(const QString &indicatorName);
const QMetaObject* getStrategyMetaObject(const QString &strategyName);

#endif // INDICATORS_AND_STRATEGIES_H
