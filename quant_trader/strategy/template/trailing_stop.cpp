#include <QDataStream>

#include "trailing_stop.h"

TrailingStop::TrailingStop(bool direction, double initStop, double AFstep, double AFmax) :
    direction(direction),
    stopLoss(initStop),
    AFstep(AFstep),
    AFmax(AFmax)
{
}

TrailingStop::TrailingStop(bool direction, double initStop) :
    TrailingStop(direction, initStop, 0.02f, 0.19999f)
{
}

TrailingStop::TrailingStop() :
    isEnabled(false)
{
}

bool TrailingStop::checkStopLoss(double price) const {
    if (isEnabled) {
        return (direction && price < stopLoss) || (!direction && price > stopLoss);
    } else {
        return false;
    }
}

void TrailingStop::update(double highPrice, double lowPrice) {
    if (!isEnabled) {
        return;
    }

    if (isNewCreate) {
        // First time, only record the high/low price, don't update AF and SL
        if (direction) {
            highestEver = highPrice;
        } else {
            lowestEver = lowPrice;
        }
        isNewCreate = false;
        return;
    }

    if (direction) {
        if (highPrice > highestEver) {
            highestEver = highPrice;
            if (AF < AFmax) {
                AF += AFstep;
            }
        }
        stopLoss = stopLoss + (highestEver - stopLoss) * AF;
    } else {
        if (lowPrice < lowestEver) {
            lowestEver = lowPrice;
            if (AF < AFmax) {
                AF += AFstep;
            }
        }
        stopLoss = stopLoss - (stopLoss - lowestEver) * AF;
    }
}

QDebug operator<<(QDebug dbg, const TrailingStop &stop)
{
    dbg.nospace() <<   "direction="   << stop.direction
                  << ", stopLoss="    << stop.stopLoss
                  << ", AFstep="      << stop.AFstep
                  << ", AFmax="       << stop.AFmax
                  << ", NewCreate="   << stop.isNewCreate
                  << ", Enabled="     << stop.isEnabled
                  << ", AF="          << stop.AF
                  << (stop.direction ? ", highestEver=" : ", lowestEver=")
                  << (stop.direction ? stop.highestEver : stop.lowestEver);
    return dbg.space();
}

QDataStream &operator<<(QDataStream &s, const TrailingStop &stop)
{
    s << stop.direction;
    s << stop.stopLoss;
    s << stop.AFstep;
    s << stop.AFmax;
    s << stop.isNewCreate;
    s << stop.isEnabled;
    s << stop.AF;
    s << stop.highestEver;
    s << stop.lowestEver;
    return s;
}

QDataStream &operator>>(QDataStream &s, TrailingStop &stop)
{
    s >> stop.direction;
    s >> stop.stopLoss;
    s >> stop.AFstep;
    s >> stop.AFmax;
    s >> stop.isNewCreate;
    s >> stop.isEnabled;
    s >> stop.AF;
    s >> stop.highestEver;
    s >> stop.lowestEver;
    return s;
}
