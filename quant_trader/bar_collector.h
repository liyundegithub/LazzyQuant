#ifndef BAR_COLLECTOR_H
#define BAR_COLLECTOR_H

#include <QObject>
#include <QMap>
#include <QSqlDatabase>

#include "bar.h"

class BarCollector : public QObject
{
    Q_OBJECT
    Q_ENUMS(TimeFrame)

public:
    enum TimeFrame {
        SEC3  = 0x0001,
        SEC5  = 0x0002,
        SEC6  = 0x0004,
        SEC10 = 0x0008,
        SEC12 = 0x0010,
        SEC15 = 0x0020,
        SEC20 = 0x0040,
        SEC30 = 0x0080,
        MIN1  = 0x0100,
        MIN3  = 0x0200,
        MIN5  = 0x0400,
        MIN10 = 0x0800,
        MIN15 = 0x1000,
        MIN30 = 0x2000,
        MIN60 = 0x4000,
        DAY   = 0x8000,
    };
    Q_DECLARE_FLAGS(TimeFrames, TimeFrame)

    explicit BarCollector(const QString &instrumentID, const TimeFrames &timeFrameFlags, bool saveBarsToDB, QObject *parent = 0);
    ~BarCollector();

    Bar *getBarPtr(int timeFrame) {
        return &barMap[timeFrame];
    }
    bool onMarketData(int time, double lastPrice, int volume);
    void saveEmitReset(int timeFrame, Bar &bar);

protected:
    const QString instrument;
    bool saveBarsToDB;
    QSqlDatabase sqlDB;

    int lastVolume = 0;
    qint64 lastNightBase = 0;
    qint64 morningBase = 0;
    qint64 tradingDayBase = 0;

    QList<int> keys;
    QMap<int, Bar> barMap;

signals:
    void collectedBar(const QString &instrumentID, int timeFrame, const Bar &bar);
public slots:
    void setTradingDay(const QString &tradingDay, const QString &lastNight);
    void saveBar(int timeFrame, const Bar &bar);
    void flush();
};

#endif // BAR_COLLECTOR_H
