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
    QList<Bar>* getBars(const QString &instrumentID, const QString &time_frame_str);

    MultipleTimer *multiTimer;
    QList<QTime> saveBarTimePoints;
    QList<QList<BarCollector*>> collectorsToSave;

public:
    explicit QuantTrader(QObject *parent = 0);
    ~QuantTrader();

    static QuantTrader *instance;
    AbstractIndicator* registerIndicator(const QString &instrumentID, const QString &time_frame_str, QString indicator_name, ...);

private slots:
    void onNewBar(const QString &instrumentID, int time_frame, const Bar& bar);
    void timesUp(int index);

signals:

public slots:
    void onMarketData(const QString& instrumentID, uint time, double lastPrice, int volume,
                      double askPrice1, int askVolume1, double bidPrice1, int bidVolume1);
};

#endif // QUANT_TRADER_H
