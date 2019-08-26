#ifndef TRADING_CALENDAR_H
#define TRADING_CALENDAR_H

#include <QSet>
#include <QDate>

class TradingCalendar
{
    TradingCalendar();
    ~TradingCalendar() = default;

public:
    TradingCalendar(const TradingCalendar &arg) = delete;
    TradingCalendar(const TradingCalendar &&arg) = delete;
    TradingCalendar& operator=(const TradingCalendar &arg) = delete;
    TradingCalendar& operator=(const TradingCalendar &&arg) = delete;

    static TradingCalendar *getInstance();

    bool isTradingDay(const QDate &date = QDate::currentDate()) const;
    bool tradesTonight(const QDate &date = QDate::currentDate()) const;
    QDate getOpenDay(const QDate &date = QDate::currentDate()) const;

    int getTradingDays(const QDate &startDate, const QDate &endDate) const;

protected:
    QSet<QDate> nonTradingDays; // Mon ~ Fri but market close

};

#endif // TRADING_CALENDAR_H
