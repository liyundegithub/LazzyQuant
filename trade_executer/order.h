#ifndef ORDER_H
#define ORDER_H

#include <QString>
struct CThostFtdcOrderField;

enum class OrderStatus {
    UNKNOWN,
    PENDING,
    COMPLETED,
    CANCELED,
};

class Order {
public:
    QString instrument;
    double price;
    int vol;
    int volRemain;
    QByteArray refId;
    int frontId;
    int sessionId;
    QByteArray exchangeId;
    QByteArray orderSysId;
    bool direction; // true: long, false: short
    OrderStatus status;

    Order(const CThostFtdcOrderField &field);
    Order(const Order &other);

    int remainVolume() const {
        if (direction) {
            return volRemain;
        } else {
            return -volRemain;
        }
    }

    void updateStatus(const CThostFtdcOrderField &field);
    bool matchId(const QByteArray &refId, int frontId, int sessionId) const;
    bool matchId(const QByteArray &exchangeId, const QByteArray &orderSysId) const;
};

#endif // ORDER_H
