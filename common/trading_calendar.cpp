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
        nonTradingDays.append(QDate::fromString(nonTradingDaysStr, "yyyyMMdd"));
    }
    settings->endGroup();
}

bool TradingCalendar::isTradingDay(const QDate &date)
{
    const int day = date.dayOfWeek();
    if (day == 0 || day == 6 || day == 7) {
        return false;
    } else {
        if (nonTradingDays.contains(date)) {
            return false;
        }
    }
    return true;
}

bool TradingCalendar::tradesTonight(const QDate &date)
{
    if (!isTradingDay(date)) {
        return false;
    } else {
        QDate day1 = date.addDays(1);
        QDate day2 = date.addDays(2);
        QDate day3 = date.addDays(3);
        if (!isTradingDay(day1) && !isTradingDay(day2) && !isTradingDay(day3)) {
            return false;
        }
    }
    return true;
}

int TradingCalendar::getTradingDays(const QDate &startDate, const QDate &endDate)
{
    int sum = 0;
    for (QDate date = startDate; date <= endDate; date = date.addDays(1)) {
        if (isTradingDay(date)) {
            sum ++;
        }
    }
    return sum;
}
