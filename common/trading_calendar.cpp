#include "config.h"
#include "common_utility.h"
#include "trading_calendar.h"

#include <QSettings>

TradingCalendar::TradingCalendar()
{
    auto settings = getSettingsSmart(ORGANIZATION, "trading_calander");
    settings->beginGroup("NonTradingDays");
    const auto nonTradingDaysStrs = settings->childKeys();
    for (const auto &nonTradingDaysStr : nonTradingDaysStrs) {
        // TODO use bit definition to seperate different market
        nonTradingDays.insert(QDate::fromString(nonTradingDaysStr, QStringLiteral("yyyyMMdd")));
    }
    settings->endGroup();
}

TradingCalendar *TradingCalendar::getInstance()
{
    static TradingCalendar tradingCalendar;
    return &tradingCalendar;
}

bool TradingCalendar::isTradingDay(const QDate &date) const
{
    const int day = date.dayOfWeek();
    if (day == 0 || day == 6 || day == 7) {
        return false;
    } 
    return !nonTradingDays.contains(date);
}

bool TradingCalendar::tradesTonight(const QDate &date) const
{
    QDate day1 = date.addDays(1);
    QDate day2 = date.addDays(2);
    QDate day3 = date.addDays(3);
    return isTradingDay(date) && (isTradingDay(day1) || (!isTradingDay(day2) && isTradingDay(day3)));
}

QDate TradingCalendar::getOpenDay(const QDate &date) const
{
    QDate day1 = date.addDays(-1);
    QDate day2 = date.addDays(-2);
    QDate day3 = date.addDays(-3);

    if (isTradingDay(day1)) {
        return day1;
    }
    if (isTradingDay(day2)) {
        return date;
    }
    if (isTradingDay(day3)) {
        return day3;
    }
    return date;
}

int TradingCalendar::getTradingDays(const QDate &startDate, const QDate &endDate) const
{
    int sum = 0;
    for (QDate date = startDate; date <= endDate; date = date.addDays(1)) {
        if (isTradingDay(date)) {
            sum ++;
        }
    }
    return sum;
}
