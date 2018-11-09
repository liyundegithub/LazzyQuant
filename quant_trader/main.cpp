#include <QCoreApplication>
#include <QCommandLineParser>

#include "config.h"
#include "quant_trader.h"
#include "quant_trader_adaptor.h"
#include "message_handler.h"
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

    parser.addOptions({
        {{"r", "replay"},
            QCoreApplication::translate("main", "Replay mode")},
        {{"s", "start"},
            QCoreApplication::translate("main", "Replay start date"), "yyyyMMdd"},
        {{"e", "end"},
            QCoreApplication::translate("main", "Replay end date"), "yyyyMMdd"},
        {{"a", "save"},
            QCoreApplication::translate("main", "Force saving collected bars to DB even in replay mode.")},
        {{"x", "executer"},
            QCoreApplication::translate("main", "Connect to TradeExecuter, should not be used with -n")},
        {{"n", "noexecuter"},
            QCoreApplication::translate("main", "Don't connect to TradeExecuter, should not be used with -x")},
        {{"l", "savetradelog"},
            QCoreApplication::translate("main", "Save trade log to DB.")},
        {{"f", "logtofile"},
            QCoreApplication::translate("main", "Save log to a file")},
    });

    parser.process(a);
    bool replayMode = parser.isSet("replay");
    QString replayStartDate;
    QString replayEndDate;
    if (replayMode) {
        replayStartDate = parser.value("start");
        replayEndDate = parser.value("end");
    }
    bool explicitSave = parser.isSet("save");
    bool saveBarsToDB = explicitSave || (!replayMode);

    bool explicitConnectToExecuter = parser.isSet("executer");
    bool explicitNoConnectToExecuter = parser.isSet("noexecuter");
    bool saveTradeLogToDB = parser.isSet("savetradelog");

    bool log2File = parser.isSet("logtofile");
    setupMessageHandler(true, log2File, "quant_trader");

    QuantTrader quantTrader(traderConfigs[0], saveBarsToDB);
    QuantTraderManagerDbus manager(&quantTrader, replayMode, {replayStartDate, replayEndDate});
    qInfo() << "marketSource =" << manager.marketSource;

    com::lazzyquant::trade_executer *pExecuter = nullptr;
    if ((!replayMode && !explicitNoConnectToExecuter) || explicitConnectToExecuter) {
        pExecuter = new com::lazzyquant::trade_executer(EXECUTER_DBUS_SERVICE, EXECUTER_DBUS_OBJECT, QDBusConnection::sessionBus());
        quantTrader.cancelAllOrders = std::bind(&com::lazzyquant::trade_executer::cancelAllOrders, pExecuter, _1);
        quantTrader.setPosition = std::bind(&com::lazzyquant::trade_executer::setPosition, pExecuter, _1, _2);
    }

    TradeLogger *pLogger = nullptr;
    if (saveTradeLogToDB) {
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
    if (pLogger) {
        delete pLogger;
    }
    if (pExecuter) {
        delete pExecuter;
    }
    restoreMessageHandler();
    return ret;
}
