#ifndef BAR_COLLECTOR_H
#define BAR_COLLECTOR_H

#include <QObject>
#include <QMap>

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

    explicit BarCollector(const QString& instrumentID, const TimeFrames &time_frame_flags, QObject *parent = 0);
    ~BarCollector();

    static QString collector_dir;
    Bar *getCurrentBar(int timeFrame) {
        return &current_bar_map[timeFrame];
    }
    bool onMarketData(int time, double lastPrice, int volume);

protected:
    const QString instrument;
    int lastVolume = 0;
    qint64 baseSecOfDays = 0;

    QList<int> keys;
    QMap<int, QList<Bar>> bar_list_map;
    QMap<int, Bar> current_bar_map;

signals:
    void collectedBar(const QString& instrumentID, int time_frame, const Bar& bar);
public slots:
    void setTradingDay(const QString &tradingDay);
    void saveBars();
};

#endif // BAR_COLLECTOR_H
