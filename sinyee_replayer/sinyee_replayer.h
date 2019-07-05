#ifndef SINYEE_REPLAYER_H
#define SINYEE_REPLAYER_H

#include "common_replayer.h"

struct CONFIG_ITEM;

class SinYeeReplayer : public CommonReplayer
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lazzyquant.tick_replayer")

    QString sinYeeDataPath;

public:
    explicit SinYeeReplayer(const CONFIG_ITEM &config, QObject *parent = nullptr);

protected:
    void appendTicksToList(const QString &date, const QString &instrument) override;

public slots:
    bool replayTo(int time) override;

};

#endif // SINYEE_REPLAYER_H
