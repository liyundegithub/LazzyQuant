#ifndef TRADING_CALENDAR_H
#define TRADING_CALENDAR_H

#include <QSet>
#include <QDate>

class TradingCalendar
{
    TradingCalendar();

public:
    TradingCalendar(const TradingCalendar& arg) = delete; // Copy constructor
    TradingCalendar(const TradingCalendar&& arg) = delete;  // Move constructor
    TradingCalendar& operator=(const TradingCalendar& arg) = delete; // Assignment operator
    TradingCalendar& operator=(const TradingCalendar&& arg) = delete; // Move operator

    static TradingCalendar *getInstance();

    bool isTradingDay(const QDate &date = QDate::currentDate()) const;
    bool tradesTonight(const QDate &date = QDate::currentDate()) const;
    QDate getOpenDay(const QDate &date = QDate::currentDate()) const;

    int getTradingDays(const QDate &startDate, const QDate &endDate) const;

protected:
    QSet<QDate> nonTradingDays; // Mon ~ Fri but market close

};

#endif // TRADING_CALENDAR_H
