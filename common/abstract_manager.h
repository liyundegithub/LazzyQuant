#ifndef ABSTRACT_MANAGER_H
#define ABSTRACT_MANAGER_H

class QObject;

class AbstractManager
{
public:
    virtual ~AbstractManager() {}

    virtual QObject *getSource() const = 0;
    virtual QObject *getTrader() const = 0;
    virtual QObject *getExecuter() const = 0;
    virtual void init() = 0;

};

#endif // ABSTRACT_MANAGER_H
