#include "common_tick.h"
#include "sinyee_tick.h"

#include <QTime>
#include <QDataStream>
#include <QDebugStateSaver>

SinYeeTick::operator CommonTick()
{
    CommonTick commonTick;
    commonTick.setTimeStamp(time, msec);
    commonTick.price = price;
    commonTick.askPrice = askPrice;
    commonTick.bidPrice = bidPrice;
    commonTick.volume = volume;
    commonTick.askVolume = askVolume;
    commonTick.bidVolume = bidVolume;
    return commonTick;
}

QStringList SinYeeTick::getAvailableContracts(QDataStream& tickStream)
{
    qint16 num;
    tickStream >> num;
    Q_ASSERT(num >= 0);

    char buf[32];
    QStringList contracts;

    for (int i = 0; i < num; i ++) {
        qint8 strlen;
        tickStream >> strlen;
        Q_ASSERT(strlen > 0);

        memset(buf, 0, 32);
        tickStream.readRawData(buf, strlen);
        QString contract(buf);
        contracts << contract;

        tickStream.skipRawData(29 - strlen);
    }

    return contracts;
}

QList<SinYeeTick> SinYeeTick::readTicks(QDataStream& tickStream, int num)
{
    QList<SinYeeTick> tickList;
    for (int i = 0; i < num; i++) {
        SinYeeTick tick;
        tickStream >> tick;
        tickList << tick;
    }
    return tickList;
}

QDebug operator<<(QDebug dbg, const SinYeeTick &tick)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Ask 1:\t" << tick.askPrice << '\t' << tick.askVolume << '\n'
                  << " ------ " << QTime(0, 0).addSecs(tick.time).toString() <<  " lastPrice:" << tick.price << " ------ " << '\n'
                  << "Bid 1:\t" << tick.bidPrice << '\t' << tick.bidVolume;
    return dbg;
}

QDataStream& operator>>(QDataStream& s, SinYeeTick& dataTick)
{
    s >> dataTick.time;
    s >> dataTick.msec;
    s >> dataTick.price;
    s >> dataTick.volume;
    s >> dataTick.bidPrice;
    s >> dataTick.bidVolume;
    s >> dataTick.askPrice;
    s >> dataTick.askVolume;
    s >> dataTick.openInterest;
    s >> dataTick.direction;
    return s;
}
