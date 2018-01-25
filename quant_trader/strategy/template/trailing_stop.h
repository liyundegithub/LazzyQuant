#ifndef TRAILING_STOP_H
#define TRAILING_STOP_H

#include <cfloat>
#include <QDebug>

class TrailingStop
{
    bool direction;
    double stopLoss;
    double AFstep, AFmax;
    bool isNewCreate = true;
    bool isEnabled = true;

    double AF = 0.0;
    double highestEver = -DBL_MAX;
    double lowestEver = DBL_MAX;

public:
    TrailingStop(bool direction, double initStop, double AFstep, double AFmax);
    TrailingStop(bool direction, double initStop);
    TrailingStop();

    bool getDirection() const { return direction; }
    bool getEnabled() const { return isEnabled; }
    void enable() { isEnabled = true; }
    void disable() { isEnabled = false; }

    bool checkStopLoss(double price) const;
    void update(double highPrice, double lowPrice);

    friend QDebug operator<<(QDebug dbg, const TrailingStop &stop);
    friend QDataStream &operator<<(QDataStream &s, const TrailingStop &stop);
    friend QDataStream &operator>>(QDataStream &s, TrailingStop &stop);
};


#endif // TRAILING_STOP_H
