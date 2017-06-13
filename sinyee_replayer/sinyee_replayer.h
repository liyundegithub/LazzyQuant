#ifndef SINYEE_REPLAYER_H
#define SINYEE_REPLAYER_H

#include <QObject>
#include <QStringList>

struct CONFIG_ITEM;

class SinYeeReplayer : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lazzyquant.sinyee_replayer")

    QString sinYeeDataPath;
    QStringList replayList;

public:
    explicit SinYeeReplayer(const CONFIG_ITEM &config, QObject *parent = 0);

signals:
    void newMarketData(const QString& instrumentID, uint time, double lastPrice, int volume,
                       double askPrice1, int askVolume1, double bidPrice1, int bidVolume1);

public slots:
    void startReplay(const QString &date);
    void startReplay(const QString &date, const QString &instrument);
    void startReplay(const QString &date, const QStringList &instruments);
};

#endif // SINYEE_REPLAYER_H
