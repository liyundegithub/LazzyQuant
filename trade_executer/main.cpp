#include <QCoreApplication>

#include "config.h"
#include "ctp_executer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QList<CtpExecuter*> executerList;
    for (const auto & config : executerConfigs) {
        CtpExecuter *pExecuter = new CtpExecuter(config);
        executerList.append(pExecuter);
    }

    int ret = a.exec();

    for (const auto & pExecuter : executerList) {
        delete pExecuter;
    }
    return ret;
}
