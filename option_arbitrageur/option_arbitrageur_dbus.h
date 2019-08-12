#ifndef OPTION_ARBITRAGEUR_DBUS_H
#define OPTION_ARBITRAGEUR_DBUS_H

class AbstractManager;
class OptionHelper;
struct OptionArbitrageurOptions;

class OptionArbitrageurDbus
{
    AbstractManager *pManager = nullptr;
    OptionHelper *pHelper = nullptr;

public:
    explicit OptionArbitrageurDbus(const OptionArbitrageurOptions &options);
    ~OptionArbitrageurDbus();
};

#endif // OPTION_ARBITRAGEUR_DBUS_H
