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

    int tradingDaysTo(const QDate &endDate, const QDate &startDate = QDate::currentDate());

protected:
    QList<QDate> nonTradingDays; // Mon ~ Fri but market close
};

#endif // TRADING_CALENDAR_H
