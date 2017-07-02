#include <QCoreApplication>

#include "future_arbitrageur.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    FutureArbitrageur arbitrageur;
    return a.exec();
}
