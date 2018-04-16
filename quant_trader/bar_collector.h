#ifndef BAR_COLLECTOR_H
#define BAR_COLLECTOR_H

#include <QObject>
#include <QMap>
#include <QSqlDatabase>

#include "bar.h"

class BarCollector : public QObject
{
    Q_OBJECT

public:
    enum TimeFrame {
        CURRENT = 0x00000000,
        SEC3    = 0x00000001,
        SEC5    = 0x00000002,
        SEC6    = 0x00000004,
        SEC10   = 0x00000008,
        SEC12   = 0x00000010,
        SEC15   = 0x00000020,
        SEC20   = 0x00000040,
        SEC30   = 0x00000080,
        MIN1    = 0x00000100,
        MIN2    = 0x00000200,
        MIN3    = 0x00000400,
        MIN4    = 0x00000800,
        MIN5    = 0x00001000,
        MIN6    = 0x00002000,
        MIN10   = 0x00004000,
        MIN12   = 0x00008000,
        MIN15   = 0x00010000,
        MIN20   = 0x00020000,
        MIN30   = 0x00040000,
        HOUR1   = 0x00080000,
        HOUR2   = 0x00100000,
        HOUR3   = 0x00200000,
        HOUR4   = 0x00400000,
        HOUR6   = 0x00800000,
        HOUR8   = 0x01000000,
        HOUR12  = 0x02000000,
        DAY     = 0x04000000,
        WEEK    = 0x08000000,
        MONTH   = 0x10000000,
    };
    Q_DECLARE_FLAGS(TimeFrames, TimeFrame)
    Q_FLAG(TimeFrames)

    explicit BarCollector(const QString &instrumentID, int timeFrameFlags, bool saveBarsToDB, QObject *parent = 0);
    ~BarCollector();

    Bar *getBarPtr(int timeFrame) {
        return &barMap[timeFrame];
    }
    void setTradingDay(const QString &tradingDay, const QString &lastNight);
    bool onMarketData(int time, double lastPrice, int volume);

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

    void saveEmitReset(int timeFrame, Bar &bar);
    void saveBar(int timeFrame, const Bar &bar);

signals:
    void collectedBar(const QString &instrumentID, int timeFrame, const Bar &bar);

public slots:
    void flush(bool endOfDay);
};

#endif // BAR_COLLECTOR_H
