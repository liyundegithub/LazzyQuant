#ifndef ABSTRACT_MANAGER_H
#define ABSTRACT_MANAGER_H

class QObject;

class AbstractManager
{
protected:
    QObject *pDataSource = nullptr;
    QObject *pTrader = nullptr;
    QObject *pExecuter = nullptr;

public:
    QObject *getDataSource() const { return pDataSource; }
    QObject *getTrader() const { return pTrader; }
    QObject *getExecuter() const { return pExecuter; }

};

#endif // ABSTRACT_MANAGER_H
