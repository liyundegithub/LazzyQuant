#ifndef EDITABLE_H
#define EDITABLE_H

#include <QSqlTableModel>

class Editable {
public:
    QString signature;
    QSqlTableModel model;

    virtual void setup();
    virtual void loadFromDB();
};

#endif // EDITABLE_H
