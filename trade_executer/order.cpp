#include "order.h"

#include "ThostFtdcUserApiStruct.h"

static inline OrderStatus parseCTPOrderStatus(TThostFtdcOrderStatusType ctpStatus)
{
    OrderStatus orderStatus;

    switch(ctpStatus) {
    case THOST_FTDC_OST_AllTraded:
        orderStatus = OrderStatus::COMPLETED;
        break;
    case THOST_FTDC_OST_PartTradedQueueing:
    case THOST_FTDC_OST_PartTradedNotQueueing:  // FIXME
    case THOST_FTDC_OST_NoTradeQueueing:
    case THOST_FTDC_OST_NoTradeNotQueueing:     // FIXME
        orderStatus = OrderStatus::PENDING;
        break;
    case THOST_FTDC_OST_Canceled:
        orderStatus = OrderStatus::CANCELED;
        break;
    case THOST_FTDC_OST_Unknown:
        // same as default
    default:
        orderStatus = OrderStatus::UNKNOWN;
        break;
    }

    return orderStatus;
}

Order::Order(const CThostFtdcOrderField &field)
{
    instrument = field.InstrumentID;
    price = field.LimitPrice;
    vol = field.VolumeTotalOriginal;
    volRemain = field.VolumeTotal;
    refId = field.OrderRef;
    frontId = field.FrontID;
    sessionId = field.SessionID;
    exchangeId = field.ExchangeID;
    orderSysId = field.OrderSysID;
    direction = (field.Direction == THOST_FTDC_D_Buy);
    status = parseCTPOrderStatus(field.OrderStatus);
}

Order::Order(const Order &other)
{
    instrument = other.instrument;
    price = other.price;
    vol = other.vol;
    volRemain = other.volRemain;
    refId = other.refId;
    frontId = other.frontId;
    sessionId = other.sessionId;
    exchangeId = other.exchangeId;
    orderSysId = other.orderSysId;
    direction = other.direction;
    status = other.status;
}

void Order::updateStatus(const CThostFtdcOrderField &field)
{
    volRemain = field.VolumeTotal;
    status = parseCTPOrderStatus(field.OrderStatus);
}

bool Order::matchId(const QByteArray &refId, int frontId, int sessionId) const
{
    return (refId == this->refId && frontId == this->frontId && sessionId == this->sessionId);
}

bool Order::matchId(const QByteArray &exchangeId, const QByteArray &orderSysId) const
{
    return (exchangeId == this->exchangeId && orderSysId == this->orderSysId);
}
