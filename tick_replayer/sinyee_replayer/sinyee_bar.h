#ifndef SINYEE_BAR_H
#define SINYEE_BAR_H

#include <QDebug>

class QDataStream;

struct SinYeeBar
{
    int time;
    float open;
    float high;
    float low;
    float close;
    float settlement;
    float volume;
    float openInterest;

    static QMultiMap<QString, QPair<int, int>> getAvailableContracts(QDataStream& barStream);
    static QList<SinYeeBar> readBars(QDataStream& barStream, int num);
};

QDebug operator<<(QDebug dbg, const SinYeeBar &bar);
QDataStream& operator>>(QDataStream& s, SinYeeBar& bar);

#endif // SINYEE_BAR_H
