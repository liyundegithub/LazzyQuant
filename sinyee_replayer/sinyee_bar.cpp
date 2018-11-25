#include "sinyee_bar.h"

#include <QTime>
#include <QDataStream>
#include <QDebugStateSaver>

QMultiMap<QString, QPair<int, int>> SinYeeBar::getAvailableContracts(QDataStream& barStream)
{
    qint16 num;
    barStream >> num;
    Q_ASSERT(num >= 0);

    char buf[32];
    QMultiMap<QString, QPair<int, int>> contracts;

    for (int i = 0; i < num; i ++) {
        qint8 strlen;
        barStream >> strlen;
        Q_ASSERT(strlen > 0);

        memset(buf, 0, 32);
        barStream.readRawData(buf, strlen);
        QString contract(buf);

        barStream.skipRawData(29 - strlen);

        for (int j = 0; j < 8; j++) {
            int num1, num2;
            barStream >> num1;
            barStream >> num2;
            auto offsetNums = qMakePair(num1, num2);
            contracts.insert(contract, offsetNums);
        }
    }

    return contracts;
}

QList<SinYeeBar> SinYeeBar::readBars(QDataStream& barStream, int num)
{
    QList<SinYeeBar> tickList;
    for (int i = 0; i < num; i++) {
        SinYeeBar bar;
        barStream >> bar;
        tickList << bar;
    }
    return tickList;
}

QDebug operator<<(QDebug dbg, const SinYeeBar &bar)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << QTime(0, 0).addSecs(bar.time).toString()
                  << ", open = " << bar.open
                  << ", high = " << bar.high
                  << ", low = " << bar.low
                  << ", close = " << bar.close
                  << ", settlement = " << bar.settlement
                  << ", volume = " << bar.volume
                  << ", openInterest = " << bar.openInterest;
    return dbg;
}

QDataStream& operator>>(QDataStream& s, SinYeeBar& bar)
{
    s >> bar.time;
    s >> bar.open;
    s >> bar.high;
    s >> bar.low;
    s >> bar.close;
    s >> bar.settlement;
    s >> bar.volume;
    s >> bar.openInterest;
    return s;
}
