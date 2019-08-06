#ifndef CTP_REPLAYER_H
#define CTP_REPLAYER_H

#include "ThostFtdcUserApiStruct.h"

#include "time_mapper.h"
#include "tick_replayer.h"

struct CONFIG_ITEM;

class CtpReplayer : public TickReplayer
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lazzyquant.tick_replayer")

    QString marketDataPath;
    TimeMapper mapTime;
    QList<CThostFtdcDepthMarketDataField> ctpMdList;

public:
    explicit CtpReplayer(const CONFIG_ITEM &config, QObject *parent = nullptr);

protected:
    void appendTicksToList(const QString &date, const QString &instrument) override;
    void sortTickPairList() override;

};

#endif // CTP_REPLAYER_H
