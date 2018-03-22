#include "sinyee_tick.h"

#include <QTime>
#include <QDataStream>
#include <QDebugStateSaver>

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
