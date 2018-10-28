#ifndef SINYEE_REPLAYER_H
#define SINYEE_REPLAYER_H

#include <QObject>
#include <QStringList>

#include "sinyee_tick.h"
#include "time_mapper.h"

struct CONFIG_ITEM;

class SinYeeReplayer : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lazzyquant.sinyee_replayer")

    TimeMapper mapTime;

    QString sinYeeDataPath;
    QStringList replayList;

    QString replayDate;
    QList<QPair<QString, SinYeeTick>> tickPairList;
    int tickCnt = 0;
    int replayIdx = 0;
    QMap<QString, int> sumVol;

public:
    explicit SinYeeReplayer(const CONFIG_ITEM &config, QObject *parent = nullptr);

private:
    void appendTicksToList(const QString &date, const QString &instrument);
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
    bool replayTo(int time);
};

#endif // SINYEE_REPLAYER_H
