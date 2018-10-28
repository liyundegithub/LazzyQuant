#include "time_mapper.h"
#include "trading_calendar.h"

#include <QString>
#include <QDateTime>
#include <QTimeZone>

TimeMapper::TimeMapper()
{
    pTradingCalendar = new TradingCalendar;
}

TimeMapper::~TimeMapper()
{
    delete pTradingCalendar;
}

void TimeMapper::setTradingDay(const QString &tradingDay)
{
    auto tradingDateTime = QDateTime::fromString(tradingDay, QStringLiteral("yyyyMMdd"));
    tradingDateTime.setTimeZone(QTimeZone::utc());
    auto newTradingDayBase = tradingDateTime.toSecsSinceEpoch();
    if (tradingDayBase != newTradingDayBase) {
        tradingDayBase = newTradingDayBase;

        QDate date = tradingDateTime.date();
        do {
            date = date.addDays(-1);
        } while (!pTradingCalendar->isTradingDay(date));

        QDateTime lastTradingDateTime(date);
        lastTradingDateTime.setTimeZone(QTimeZone::utc());
        lastNightBase = lastTradingDateTime.toSecsSinceEpoch();
        morningBase = lastNightBase + 24 * 3600;
    }
}

qint64 TimeMapper::mapTime(int inTime) const
{
    qint64 outTime = 0;
    if (inTime < 8 * 3600) {
        outTime = morningBase + inTime;
    } else if (inTime > 17 * 3600) {
        outTime = lastNightBase + inTime;
    } else {
        outTime = tradingDayBase + inTime;
    }
    return outTime;
}
