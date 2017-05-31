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
    int refId;
    int frontId;
    int sessionId;
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
};

#endif // ORDER_H
