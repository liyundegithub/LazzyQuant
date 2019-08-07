#include <QTimeZone>
#include <QCoreApplication>
#include <QCommandLineParser>

#include "config.h"
#include "message_handler.h"
#include "trade_logger.h"
#include "market_watcher.h"
#include "sinyee_replayer.h"
#include "parked_order.h"
#include "ctp_executer.h"
#include "quant_trader_options.h"
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

    qMetaTypeId<ParkedOrder>();

    QCommandLineParser parser;
    parser.setApplicationDescription("Quant trader bundled with market watcher, tick replayer and trade executer.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions(quantTraderOptions);
    parser.addOption({{"c", "source"}, "main", "Market source", "ctp/sinyee"});

    parser.process(a);
    QuantTraderOptions options = getQuantTraderOptions(parser);
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

    setupMessageHandler(true, options.log2File, "quant_trader");

    QuantTrader quantTrader(traderConfigs[0], options.saveBarsToDB());

    CtpExecuter *pExecuter = nullptr;
    if ((!options.replayMode && !options.explicitNoConnectToExecuter) || options.explicitConnectToExecuter) {
        pExecuter = new CtpExecuter(executerConfigs[0]);
        quantTrader.cancelAllOrders = std::bind(&CtpExecuter::cancelAllOrders, pExecuter, _1);
        quantTrader.setPosition = std::bind(&CtpExecuter::setPosition, pExecuter, _1, _2);
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

    QuantTraderManagerBundle *manager = nullptr;
    switch (source) {
    case CTP:
        manager = new QuantTraderManagerBundle(
                      new MarketWatcher(watcherConfigs[0], options.replayMode),
                      &quantTrader,
                      pExecuter,
                      options.replayMode,
                      options.replayRange(),
                      true
                  );
        break;
    case SINYEE:
        manager = new QuantTraderManagerBundle(
                      new SinYeeReplayer(replayerConfigs[0]),
                      &quantTrader,
                      pExecuter,
                      options.replayRange(),
                      true
                  );
        break;
    }

    int ret = a.exec();
    delete pLogger;
    delete manager;
    restoreMessageHandler();
    return ret;
}
