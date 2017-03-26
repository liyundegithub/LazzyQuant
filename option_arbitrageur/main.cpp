#include <QCoreApplication>

#include "option_arbitrageur.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    OptionArbitrageur arbitrageur;
    return a.exec();
}
