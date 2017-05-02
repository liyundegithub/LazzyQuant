#ifndef TRADING_CALENDAR_H
#define TRADING_CALENDAR_H

#include <QList>
#include <QDate>

class TradingCalendar
{
public:
    TradingCalendar();

    bool isTradingDay(const QDate &date = QDate::currentDate());
    bool tradesTonight(const QDate &date = QDate::currentDate());

    int getTradingDays(const QDate &startDate, const QDate &endDate);

protected:
    QList<QDate> nonTradingDays; // Mon ~ Fri but market close
};

#endif // TRADING_CALENDAR_H
