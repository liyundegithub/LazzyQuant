#include <QCoreApplication>
#include <QCommandLineParser>

#include "config.h"
#include "message_handler.h"
#include "quant_trader_options.h"
#include "quant_trader.h"
#include "quant_trader_adaptor.h"
#include "trade_logger.h"
#include "quant_trader_manager_dbus.h"

#include "trade_executer_interface.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("quant_trader");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription("Collect K lines, calculate indicators/strategies and call TradeExecuter automatically.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions(quantTraderOptions);
    parser.process(a);

    auto options = getQuantTraderOptions(parser);
    setupMessageHandler(true, options.log2File, "quant_trader");

    QuantTrader quantTrader(traderConfigs[0], options.saveBarsToDB());
    QuantTraderManagerDbus manager(&quantTrader, options.replayMode, options.replayRange());
    qInfo() << "marketSource =" << manager.marketSource;

    com::lazzyquant::trade_executer *pExecuter = nullptr;
    if ((!options.replayMode && !options.explicitNoConnectToExecuter) || options.explicitConnectToExecuter) {
        pExecuter = new com::lazzyquant::trade_executer(EXECUTER_DBUS_SERVICE, EXECUTER_DBUS_OBJECT, QDBusConnection::sessionBus());
        quantTrader.cancelAllOrders = std::bind(&com::lazzyquant::trade_executer::cancelAllOrders, pExecuter, _1);
        quantTrader.setPosition = std::bind(&com::lazzyquant::trade_executer::setPosition, pExecuter, _1, _2);
    }

    TradeLogger *pLogger = nullptr;
    if (options.saveTradeLogToDB) {
        pLogger = new TradeLogger("quant_trader_" + QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMddhhmmss")));
        quantTrader.logTrade = std::bind(&TradeLogger::positionChanged, pLogger, _1, _2, _3, _4);
    } else {
        quantTrader.logTrade = [](qint64 time, const QString &instrumentID, int newPosition, double price) -> void {
            qInfo().noquote() << QDateTime::fromSecsSinceEpoch(time, QTimeZone::utc()).toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))
                              << "New position for" << instrumentID << newPosition << ", price =" << price;
        };
    }

    new Quant_traderAdaptor(&quantTrader);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(traderConfigs[0].dbusObject, &quantTrader);
    dbus.registerService(traderConfigs[0].dbusService);

    int ret = a.exec();
    delete pLogger;
    delete pExecuter;
    restoreMessageHandler();
    return ret;
}
