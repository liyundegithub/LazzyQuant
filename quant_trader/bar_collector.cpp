#include <QDataStream>
#include <QDebug>
#include <QMetaEnum>
#include <QDateTime>
#include <QTimeZone>
#include <QFile>
#include <QDir>

#include "bar_collector.h"

extern int timeFrameEnumIdx;
QString BarCollector::collector_dir;

BarCollector::BarCollector(const QString& instrumentID, const TimeFrames &time_frame_flags, QObject *parent) :
    QObject(parent),
    instrument(instrumentID)
{
    const int timeFrameEnumCount = BarCollector::staticMetaObject.enumerator(timeFrameEnumIdx).keyCount();
    for (int i = 0; i < timeFrameEnumCount; i++) {
        const auto flag = BarCollector::staticMetaObject.enumerator(timeFrameEnumIdx).value(i);
        if (time_frame_flags.testFlag(static_cast<TimeFrame>(flag))) {
            bar_list_map.insert(flag, QList<Bar>());
            current_bar_map.insert(flag, Bar());

            // Check if the directory is already created for collected bars
            QString time_frame_str = BarCollector::staticMetaObject.enumerator(timeFrameEnumIdx).valueToKey(flag);
            QString path_for_this_key = collector_dir + "/" + instrument + "/" + time_frame_str;
            QDir dir(path_for_this_key);
            if (!dir.exists()) {
                bool ret = dir.mkpath(path_for_this_key);
                if (!ret) {
                    qCritical() << "Create directory failed!";
                }
            }

            keys << flag;
        }
    }
}

BarCollector::~BarCollector()
{
    saveBars();
}

#define MIN_UNIT    60
#define HOUR_UNIT   3600

static const QMap<BarCollector::TimeFrame, int> g_time_table = {
    {BarCollector::SEC3,   3},
    {BarCollector::SEC5,   5},
    {BarCollector::SEC6,   6},
    {BarCollector::SEC10, 10},
    {BarCollector::SEC12, 12},
    {BarCollector::SEC15, 15},
    {BarCollector::SEC20, 20},
    {BarCollector::SEC30, 30},
    {BarCollector::MIN1,   1 * MIN_UNIT},
    {BarCollector::MIN3,   3 * MIN_UNIT},
    {BarCollector::MIN5,   5 * MIN_UNIT},
    {BarCollector::MIN10, 10 * MIN_UNIT},
    {BarCollector::MIN15, 15 * MIN_UNIT},
    {BarCollector::MIN30, 30 * MIN_UNIT},
    {BarCollector::MIN60,  1 * HOUR_UNIT},
    {BarCollector::DAY,   24 * HOUR_UNIT},
};

bool BarCollector::onMarketData(int time, double lastPrice, int volume)
{
    const bool isNewTick = (volume != lastVolume);
    qint64 currentTime = baseSecOfDays + time;

    for (const auto key : qAsConst(keys)) {
        Bar & bar = current_bar_map[key];
        auto time_unit = g_time_table[static_cast<TimeFrame>(key)];  // TODO optimize, use time_unit as key

        if ((currentTime / time_unit) != (bar.time / time_unit)) {
            if (bar.tick_volume > 0) {
                bar_list_map[key].append(bar);
                emit collectedBar(instrument, key, bar);
                qDebug().noquote() << instrument << ":" << bar;
                bar.init();
            }
        }

        if (!isNewTick) {
            continue;
        }

        if (bar.isNewBar()) {
            bar.open = lastPrice;
            bar.time = currentTime / time_unit * time_unit;
        }

        if (lastPrice > bar.high) {
            bar.high = lastPrice;
        }

        if (lastPrice < bar.low) {
            bar.low = lastPrice;
        }

        bar.close = lastPrice;
        bar.tick_volume ++;
        bar.volume += (volume - lastVolume);
    }
    lastVolume = volume;
    return isNewTick;
}

void BarCollector::setTradingDay(const QString& tradingDay)
{
    auto date = QDateTime::fromString(tradingDay, "yyyyMMdd");
    date.setTimeZone(QTimeZone::utc());
    auto newSecOfDays = date.toSecsSinceEpoch();
    if (baseSecOfDays != newSecOfDays) {
        baseSecOfDays = newSecOfDays;
        lastVolume = 0;
    }
}

void BarCollector::saveBars()
{
    for (const auto key : qAsConst(keys)) {
        auto & barList = bar_list_map[key];
        const auto & lastBar = current_bar_map[key];

        if (!lastBar.isNewBar()) {
            barList.append(lastBar);
        }
        if (barList.size() == 0) {
            continue;
        }
        QString time_frame_str = BarCollector::staticMetaObject.enumerator(timeFrameEnumIdx).valueToKey(key);
        QString file_name = collector_dir + "/" + instrument + "/" + time_frame_str + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz") + ".bars";
        QFile barFile(file_name);
        barFile.open(QFile::WriteOnly);
        QDataStream wstream(&barFile);
        wstream.setFloatingPointPrecision(QDataStream::DoublePrecision);
        wstream << barList;
        barFile.close();
        barList.clear();
    }
}
