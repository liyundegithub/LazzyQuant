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
    sscanf(field.OrderRef, "%12d", &refId);
    frontId = field.FrontID;
    sessionId = field.SessionID;
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
    direction = other.direction;
    status = other.status;
}

void Order::updateStatus(const CThostFtdcOrderField &field)
{
    volRemain = field.VolumeTotal;
    status = parseCTPOrderStatus(field.OrderStatus);
}
