#ifndef PARKED_ORDER_H
#define PARKED_ORDER_H

#include <QMetaType>
#include <QString>
#include <QDebug>

class QDBusArgument;
class CThostFtdcParkedOrderField;

class ParkedOrder
{
public:
    enum Status {
        UNKOWN,
        NOTSEND,
        SENT,
        DELETED,
    };

    ParkedOrder() = default;
    ParkedOrder(QByteArray id,
                Status status,
                QString instrument,
                double price,
                int volume,
                bool direction
                );

    friend QDBusArgument &operator<<(QDBusArgument &argument, const ParkedOrder &order);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, ParkedOrder &order);
    friend QDebug operator<<(QDebug dbg, const ParkedOrder &order);

    QByteArray getId() const;
    Status getStatus() const;
    QString getInstrument() const;
    double getPrice() const;
    int getVolume() const;
    bool getDirection() const;

    static ParkedOrder fromCtp(const CThostFtdcParkedOrderField &field);

protected:
    QByteArray id;
    Status status;
    QString instrument;
    double price;
    int volume;
    bool direction; // true: long, false: short

};

Q_DECLARE_METATYPE(ParkedOrder)

#endif // PARKED_ORDER_H
