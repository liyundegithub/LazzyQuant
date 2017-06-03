#include "sinyee_tick.h"

#include <QTime>

SinYeeTick::SinYeeTick()
{

}

QDebug operator<<(QDebug dbg, const SinYeeTick &tick)
{
    dbg.nospace() << "Ask 1:\t" << tick.askPrice << '\t' << tick.askVolume << '\n'
                  << " ------ " << QTime(0, 0).addSecs(tick.time).toString() <<  " lastPrice:" << tick.price << " ------ " << '\n'
                  << "Bid 1:\t" << tick.bidPrice << '\t' << tick.bidVolume;
    return dbg.space();
}
