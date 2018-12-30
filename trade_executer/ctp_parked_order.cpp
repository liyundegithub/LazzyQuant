#include "ThostFtdcUserApiStruct.h"
#include "parked_order.h"

#include <QMap>

const QMap<TThostFtdcParkedOrderStatusType, ParkedOrder::Status> ctpParkedOrderStatusMap = {
    {THOST_FTDC_PAOS_NotSend, ParkedOrder::NOTSEND},
    {THOST_FTDC_PAOS_Send,    ParkedOrder::SENT},
    {THOST_FTDC_PAOS_Deleted, ParkedOrder::DELETED},
};

const QMap<TThostFtdcDirectionType, bool> ctpDirectionMap = {
    {THOST_FTDC_D_Buy,  true},
    {THOST_FTDC_D_Sell, false},
};

ParkedOrder ParkedOrder::fromCtp(const CThostFtdcParkedOrderField &field)
{
    return {field.ParkedOrderID, ctpParkedOrderStatusMap[field.Status], field.InstrumentID,
            field.LimitPrice, field.VolumeTotalOriginal, ctpDirectionMap[field.Direction]};
}
