#ifndef SINYEE_TICK_H
#define SINYEE_TICK_H

#include <QDebug>

class SinYeeTick
{
public:
    SinYeeTick();

    int time;
    qint16 msec;
    float price;
    float volume;
    float bidPrice;
    float bidVolume;
    float askPrice;
    float askVolume;
    float openInterest;
    quint8 direction;

};

QDebug operator<<(QDebug dbg, const SinYeeTick &tick);

#endif // SINYEE_TICK_H
