#ifndef COMMON_TICK_H
#define COMMON_TICK_H

#include <QtGlobal>

struct CommonTick
{
    qint64 timestamp;   // qint64 time << 16 | qint16 msec
    double price;
    double askPrice;
    double bidPrice;
    int volume;
    int askVolume;
    int bidVolume;

    qint64 getTime() const { return timestamp >> 16; }
    void setTimeStamp(qint64 time, qint16 msec) { this->timestamp = time << 16 | msec; }

};

#endif // COMMON_TICK_H
