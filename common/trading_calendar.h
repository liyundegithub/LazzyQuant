#ifndef TRADING_CALENDAR_H
#define TRADING_CALENDAR_H

#include <QList>
#include <QDate>

class TradingCalendar
{
    TradingCalendar();

public:
    static TradingCalendar *getInstance();

    bool isTradingDay(const QDate &date = QDate::currentDate());
    bool tradesTonight(const QDate &date = QDate::currentDate());
    QDate getOpenDay(const QDate &date = QDate::currentDate());

    int getTradingDays(const QDate &startDate, const QDate &endDate);

protected:
    static TradingCalendar *instance;
    QList<QDate> nonTradingDays; // Mon ~ Fri but market close

};

#endif // TRADING_CALENDAR_H
