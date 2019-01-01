#include "parked_order.h"

#include <QDebugStateSaver>

ParkedOrder::ParkedOrder(QByteArray id,
                         Status status,
                         QString instrument,
                         double price,
                         int vol,
                         bool direction
                         ) :
    id(id),
    status(status),
    instrument(instrument),
    price(price),
    volume(vol),
    direction(direction)
{
}

QByteArray ParkedOrder::getId() const
{
    return id;
}

ParkedOrder::Status ParkedOrder::getStatus() const
{
    return status;
}

QString ParkedOrder::getInstrument() const
{
    return instrument;
}

double ParkedOrder::getPrice() const
{
    return price;
}

int ParkedOrder::getVolume() const
{
    return volume;
}

bool ParkedOrder::getDirection() const
{
    return direction;
}

const QMap<ParkedOrder::Status, QString> statusStringMap = {
    {ParkedOrder::UNKOWN,  "Unkown"},
    {ParkedOrder::NOTSEND, "NotSend"},
    {ParkedOrder::SENT,    "Send"},
    {ParkedOrder::DELETED, "Deleted"},
};

QDebug operator<<(QDebug dbg, const ParkedOrder &order)
{
    QDebugStateSaver saver(dbg);
    dbg.noquote().space() << order.id << statusStringMap[order.status]
                          << order.instrument << (order.direction ? "Buy" : "Sell")
                          << order.price << order.volume;
    return dbg;
}
