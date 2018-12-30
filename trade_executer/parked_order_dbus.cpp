#include "parked_order.h"

#include <QDBusArgument>

QDBusArgument &operator<<(QDBusArgument &argument, const ParkedOrder &order)
{
    argument.beginStructure();
    argument << order.id;
    argument << order.status;
    argument << order.instrument;
    argument << order.price;
    argument << order.volume;
    argument << order.direction;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ParkedOrder &order)
{
    argument.beginStructure();
    argument >> order.id;
    argument >> (int&)order.status;
    argument >> order.instrument;
    argument >> order.price;
    argument >> order.volume;
    argument >> order.direction;
    argument.endStructure();

    return argument;
}
