#ifndef COMMON_REPLAYER_H
#define COMMON_REPLAYER_H

#include <QObject>
#include <QStringList>
#include <QMap>

#include "common_tick.h"
#include "time_mapper.h"

class CommonReplayer : public QObject
{
    Q_OBJECT

protected:
    QStringList replayList;
    TimeMapper mapTime;
    QList<QPair<QString, CommonTick>> tickPairList;
    int tickCnt = 0;
    int replayIdx = 0;
    QMap<QString, int> sumVol;
    QString replayDate;

public:
    explicit CommonReplayer(QObject *parent = nullptr);

protected:
    virtual void appendTicksToList(const QString &date, const QString &instrument) = 0;
    void sortTickPairList();

signals:
    void tradingDayChanged(const QString& tradingDay);
    void endOfReplay(const QString& tradingDay);
    void newMarketData(const QString& instrumentID, qint64 time, double lastPrice, int volume,
                       double askPrice1, int askVolume1, double bidPrice1, int bidVolume1);

public slots:
    void startReplay(const QString &date);
    void startReplay(const QString &date, const QString &instrument);
    void startReplay(const QString &date, const QStringList &instruments);

    bool prepareReplay(const QString &date);
    bool prepareReplay(const QString &date, const QStringList &instruments);
    virtual bool replayTo(int time);

};

#endif // COMMON_REPLAYER_H
