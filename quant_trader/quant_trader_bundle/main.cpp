#include <QTimeZone>
#include <QCoreApplication>
#include <QCommandLineParser>

#include "config.h"
#include "message_handler.h"
#include "trade_logger.h"
#include "market_watcher.h"
#include "sinyee_replayer.h"
#include "ctp_executer.h"
#include "quant_trader.h"
#include "quant_trader_manager_bundle.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName("quant_trader_bundle");
    QCoreApplication::setApplicationVersion(VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription("Collect K lines, calculate indicators/strategies and call TradeExecuter automatically.");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions({
        {{"c", "source"},
            QCoreApplication::translate("main", "Market source"), "ctp/sinyee"},
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
    QString marketSource = parser.value("source");
    enum {
        CTP,
        SINYEE,
    } source;
    if (marketSource.compare("ctp", Qt::CaseInsensitive) == 0) {
        source = CTP;
    } else if (marketSource.compare("sinyee", Qt::CaseInsensitive) == 0) {
        source = SINYEE;
    } else {
        qWarning() << "Wrong market srouce parameter:" << marketSource;
        return 1;
    }

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

    CtpExecuter *pExecuter = nullptr;
    if ((!replayMode && !explicitNoConnectToExecuter) || explicitConnectToExecuter) {
        pExecuter = new CtpExecuter(executerConfigs[0]);
        quantTrader.cancelAllOrders = std::bind(&CtpExecuter::cancelAllOrders, pExecuter, _1);
        quantTrader.setPosition = std::bind(&CtpExecuter::setPosition, pExecuter, _1, _2);
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

    QuantTraderManagerBundle *manager = nullptr;
    switch (source) {
    case CTP:
        manager = new QuantTraderManagerBundle(
                      new MarketWatcher(watcherConfigs[0], replayMode),
                      &quantTrader,
                      pExecuter,
                      replayMode,
                      {replayStartDate, replayEndDate},
                      true
                  );
        break;
    case SINYEE:
        manager = new QuantTraderManagerBundle(
                      new SinYeeReplayer(replayerConfigs[0]),
                      &quantTrader,
                      pExecuter,
                      {replayStartDate, replayEndDate},
                      true
                  );
        break;
    }

    int ret = a.exec();
    if (pLogger) {
        delete pLogger;
    }
    if (manager) {
        delete manager;
    }
    restoreMessageHandler();
    return ret;
}
