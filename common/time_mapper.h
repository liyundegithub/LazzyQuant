#ifndef TIME_MAPPER_H
#define TIME_MAPPER_H

#include <QtGlobal>

class TimeMapper
{
    qint64 lastNightBase = 0;
    qint64 morningBase = 0;
    qint64 tradingDayBase = 0;

public:
    TimeMapper() = default;

    void setTradingDay(const QString &tradingDay);
    qint64 mapTime(int inTime) const;
    qint64 operator()(int inTime) const { return mapTime(inTime); }

};

#endif // TIME_MAPPER_H
