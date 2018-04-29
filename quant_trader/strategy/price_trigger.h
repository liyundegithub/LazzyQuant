#ifndef PRICE_TRIGGER_H
#define PRICE_TRIGGER_H

#include <cfloat>
#include <QString>

class PriceTrigger {
public:
    bool direction;
    double triggerPrice;

    PriceTrigger(){}
    virtual ~PriceTrigger(){}
    virtual bool getDirection() const {
        return direction;
    }
    virtual double getTriggerPrice() const {
        return triggerPrice;
    }
    virtual bool checkTrigger(double price) const {
        return getDirection() ? getTriggerPrice() < price : getTriggerPrice() > price;
    }
    virtual QString toString() = 0;
};

class EnterSignal : public PriceTrigger {
public:
    double cancelPrice;

    EnterSignal(bool direction, double triggerPrice, double cancelPrice) :
        cancelPrice(cancelPrice)
    {
        this->direction = direction;
        this->triggerPrice = triggerPrice;
    }

    // Construct unconditional enter signal
    EnterSignal(bool direction) :
        EnterSignal((direction), (direction ? DBL_MAX : -DBL_MAX), (direction ? -DBL_MAX : DBL_MAX))
    {
    }

    QString toString() {
        QString ret = "EnterSignal ";
        ret += getDirection() ? "Up: " : "Down: ";
        ret += QString("Trigger=%1;Cancel=%2").arg(triggerPrice).arg(cancelPrice);
        return ret;
    }
};

#endif // PRICE_TRIGGER_H
