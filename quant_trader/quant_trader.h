#ifndef QUANT_TRADER_H
#define QUANT_TRADER_H

#include <boost/optional.hpp>
#include <QObject>
#include <QMap>

class MultipleTimer;

class Bar;
class BarCollector;
class AbstractIndicator;
class AbstractStrategy;

class QuantTrader : public QObject
{
    Q_OBJECT

protected:
    // Following QString keys are instumentIDs
    QMap<QString, BarCollector*> collector_map;
    QMap<QString, QMap<int, QList<Bar>>> bars_map;
    QMultiMap<QString, AbstractIndicator*> indicator_map;
    QMultiMap<QString, AbstractStrategy*> strategy_map;
    QMap<QString, boost::optional<int>> position_map;

    QString kt_export_dir;
    void loadQuantTraderSettings();
    void loadTradeStrategySettings();
    QList<Bar>* getBars(const QString &instrumentID, int timeFrame);

    MultipleTimer *multiTimer;
    QList<QTime> saveBarTimePoints;
    QList<QList<BarCollector*>> collectorsToSave;

    QString currentInstrumentID;
    int currentTimeFrame;

public:
    explicit QuantTrader(QObject *parent = 0);
    ~QuantTrader();

    AbstractIndicator* registerIndicator(const QString &instrumentID, int timeFrame, QString indicator_name, ...);

private slots:
    void timesUp(int index);

signals:

public slots:
    void setTradingDay(const QString& tradingDay);
    void onMarketData(const QString& instrumentID, int time, double lastPrice, int volume,
                      double askPrice1, int askVolume1, double bidPrice1, int bidVolume1);
    void onNewBar(const QString &instrumentID, int time_frame, const Bar& bar);
};

#endif // QUANT_TRADER_H
