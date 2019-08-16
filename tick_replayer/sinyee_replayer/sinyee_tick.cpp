#include "sinyee_tick.h"

#include <QDateTime>
#include <QDataStream>
#include <QDebugStateSaver>

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
    dbg.nospace().noquote()
            << "Ask 1:\t" << tick.askPrice << '\t' << tick.askVolume << '\n'
            << " ------ " << QDateTime::fromSecsSinceEpoch(tick.time).toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")) <<  " price:" << tick.price << " ------ \n"
            << "Bid 1:\t" << tick.bidPrice << '\t' << tick.bidVolume;
    return dbg;
}

QDataStream& operator>>(QDataStream& s, SinYeeTick& tick)
{
    s >> tick.time;
    s >> tick.msec;
    s >> tick.price;
    s >> tick.volume;
    s >> tick.bidPrice;
    s >> tick.bidVolume;
    s >> tick.askPrice;
    s >> tick.askVolume;
    s >> tick.openInterest;
    s >> tick.direction;
    return s;
}
