#include <QSettings>
#include <QString>
#include <QTimer>

#include "config.h"
#include "market.h"
#include "common_utility.h"
#include "multiple_timer.h"
#include "quant_trader.h"
#include "bar.h"
#include "bar_collector.h"
#include "indicator/ma.h"
#include "indicator/parabolicsar.h"
#include "indicator/bollinger_band.h"
#include "indicator/awesome_oscillator.h"
#include "strategy/DblMaPsar_strategy.h"
#include "strategy/bighit_strategy.h"
#include "trade_executer_interface.h"

extern QList<Market> markets;
extern com::lazzyquant::trade_executer *pExecuter;

int timeFrameEnumIdx;
int MA_METHOD_enumIdx;
int APPLIED_PRICE_enumIdx;

QuantTrader::QuantTrader(QObject *parent) :
    QObject(parent)
{
    qRegisterMetaType<int>("ENUM_MA_METHOD");
    qRegisterMetaType<int>("ENUM_APPLIED_PRICE");

    timeFrameEnumIdx = BarCollector::staticMetaObject.indexOfEnumerator("TimeFrame");
    MA_METHOD_enumIdx = IndicatorFunctions::staticMetaObject.indexOfEnumerator("ENUM_MA_METHOD");
    APPLIED_PRICE_enumIdx = IndicatorFunctions::staticMetaObject.indexOfEnumerator("ENUM_APPLIED_PRICE");

    loadCommonMarketData();
    loadQuantTraderSettings();
    loadTradeStrategySettings();

    multiTimer = new MultipleTimer(saveBarTimePoints);
    connect(multiTimer, &MultipleTimer::timesUp, this, &QuantTrader::timesUp);
}

QuantTrader::~QuantTrader()
{
    qDebug() << "~QuantTrader";
}

void QuantTrader::loadQuantTraderSettings()
{
    auto settings = getSettingsSmart(ORGANIZATION, "quant_trader");

    settings->beginGroup("HistoryPath");
    kt_export_dir = settings->value("ktexport").toString();
    BarCollector::collector_dir = settings->value("collector").toString();
    settings->endGroup();

    settings->beginGroup("Collector");
    const auto instrumentIDs = settings->childKeys();

    for (const auto &instrumentID : instrumentIDs) {
        QString combined_time_frame_string = settings->value(instrumentID).toString();
        const QStringList time_frame_stringlist = combined_time_frame_string.split('|');
        BarCollector::TimeFrames time_frame_flags;
        for (const QString &tf : time_frame_stringlist) {
            int time_frame_value = BarCollector::staticMetaObject.enumerator(timeFrameEnumIdx).keyToValue(tf.trimmed().toLatin1().constData());
            BarCollector::TimeFrame time_frame = static_cast<BarCollector::TimeFrame>(time_frame_value);
            time_frame_flags |= time_frame;
        }

        BarCollector *collector = new BarCollector(instrumentID, time_frame_flags, this);
        connect(collector, SIGNAL(collectedBar(QString,int,Bar)), this, SLOT(onNewBar(QString,int,Bar)), Qt::DirectConnection);
        collector_map[instrumentID] = collector;
        qDebug() << instrumentID << ":\t" << time_frame_flags << "\t" << time_frame_stringlist;
    }
    settings->endGroup();

    QMap<QTime, QStringList> endPointsMap;
    for (const auto &instrumentID : instrumentIDs) {
        const auto endPoints = getEndPoints(instrumentID);
        for (const auto &item : endPoints) {
            endPointsMap[item] << instrumentID;
        }
    }

    auto keys = endPointsMap.keys();
    std::sort(keys.begin(), keys.end());
    for (const auto &item : qAsConst(keys)) {
        QList<BarCollector*> collectors;
        for (const auto &instrumentID : qAsConst(endPointsMap[item])) {
            collectors << collector_map[instrumentID];
        }
        collectorsToSave << collectors;
        saveBarTimePoints << item.addSecs(120); // Save bars 2 minutes after market close
    }
}

void QuantTrader::loadTradeStrategySettings()
{
    static const QMap<QString, const QMetaObject*> meta_object_map = {
        {"DblMaPsarStrategy", &DblMaPsarStrategy::staticMetaObject},
        {"BigHitStrategy", &BigHitStrategy::staticMetaObject},
        // Register more strategies here
    };

    auto settings = getSettingsSmart(ORGANIZATION, "trade_strategy");
    const QStringList groups = settings->childGroups();
    qDebug() << groups.size() << "strategies in all.";

    for (const QString& group : groups) {
        settings->beginGroup(group);
        QString strategy_name = settings->value("strategy").toString();
        QString instrument = settings->value("instrument").toString();
        QString time_frame = settings->value("timeframe").toString();

        QVariant param1 = settings->value("param1");
        QVariant param2 = settings->value("param2");
        QVariant param3 = settings->value("param3");
        QVariant param4 = settings->value("param4");
        QVariant param5 = settings->value("param5");
        QVariant param6 = settings->value("param6");
        QVariant param7 = settings->value("param7");
        QVariant param8 = settings->value("param8");
        QVariant param9 = settings->value("param9");

        settings->endGroup();

        const QMetaObject* strategy_meta_object = meta_object_map.value(strategy_name);
        QObject *object = strategy_meta_object->newInstance(Q_ARG(QString, group), Q_ARG(QString, instrument), Q_ARG(QString, time_frame), Q_ARG(QObject*, this));
        if (object == NULL) {
            qCritical() << "Instantiating strategy" << group << "failed!";
            continue;
        }

        auto *strategy = qobject_cast<AbstractStrategy*>(object);
        if (strategy == NULL) {
            qCritical() << "Cast strategy" << group << "failed!";
            delete object;
            continue;
        }

        strategy->setParameter(param1, param2, param3, param4, param5, param6, param7, param8, param9);
        strategy->setBarList(getBars(instrument, time_frame), collector_map[instrument]->getCurrentBar(time_frame));
        strategy_map.insert(instrument, strategy);

        auto position = strategy->getPosition();
        if (!position_map.contains(instrument)) {
            position_map.insert(instrument, position);
        } else if (position.is_initialized() && position_map[instrument].is_initialized()) {
            position_map[instrument] = position_map[instrument].get() + position.get();
        }
    }
}

/*!
 * \brief getKTExportName
 * 从合约代码中提取飞狐交易师导出数据的文件名
 * 比如 cu1703 --> cu03, i1705 --> i05, CF705 --> CF05
 *      SR709 --> SR109, SR809 --> SR009
 *
 * \param instrumentID 合约代码
 * \return 从飞狐交易师导出的此合约数据的文件名
 */
static inline QString getKTExportName(const QString &instrumentID) {
    const QString code = getCode(instrumentID);
    QString month = instrumentID.right(2);
    if (code == "SR" || code == "WH" || code == "bu" || code == "a") {
        const int len = instrumentID.length();
        const QString Y = instrumentID.mid(len - 3, 1);
        const int y = Y.toInt();
        if (y % 2 == 0) {
            return code + "0" + month;
        } else {
            return code + "1" + month;
        }
    } else {
        return code + month;
    }
}

/*!
 * \brief QuantTrader::getBars
 * 获取历史K线数据, 包括从飞狐交易师导出的数据和quant_trader保存的K线数据
 *
 * \param instrumentID 合约代码
 * \param time_frame_str 时间框架(字符串)
 * \return 指向包含此合约历史K线数据的QList<Bar>指针
 */
QList<Bar>* QuantTrader::getBars(const QString &instrumentID, const QString &time_frame_str)
{
    int time_frame_value = BarCollector::staticMetaObject.enumerator(timeFrameEnumIdx).keyToValue(time_frame_str.trimmed().toLatin1().constData());
    if (bars_map.contains(instrumentID)) {
        if (bars_map[instrumentID].contains(time_frame_value)) {
            return &bars_map[instrumentID][time_frame_value];
        }
    }

    // Insert a new Bar List item in bars_map
    auto &barList = bars_map[instrumentID][time_frame_value];

    // Load KT Export Data
    const QString kt_export_file_name = kt_export_dir + "/" + time_frame_str + "/" + getKTExportName(instrumentID) + getSuffix(instrumentID);
    QFile kt_export_file(kt_export_file_name);
    if (kt_export_file.open(QFile::ReadOnly)) {
        QDataStream ktStream(&kt_export_file);
        ktStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        ktStream.setByteOrder(QDataStream::LittleEndian);
        ktStream.skipRawData(12);

        QList<KTExportBar> ktBarList;
        ktStream >> ktBarList;
        qDebug() << kt_export_file_name << ktBarList.size() << "bars";
        for (const auto &ktbar : qAsConst(ktBarList)) {
            barList.append(ktbar);
        }
        kt_export_file.close();
    }

    // Load Collector Bars
    const QString collector_bar_path = BarCollector::collector_dir + "/" + instrumentID + "/" + time_frame_str;
    QDir collector_bar_dir(collector_bar_path);
    QStringList filters;
    filters << "*.bars";
    collector_bar_dir.setNameFilters(filters);
    const QStringList entries = collector_bar_dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    QList<Bar> collectedBarList;
    for (const QString &barfilename : entries) {
        QFile barfile(collector_bar_path + "/" + barfilename);
        if (!barfile.open(QFile::ReadOnly)) {
            qCritical() << "Open file:" << (collector_bar_path + "/" + barfilename) << "failed!";
            continue;
        }
        QDataStream barStream(&barfile);
        barStream.setFloatingPointPrecision(QDataStream::DoublePrecision);
        QList<Bar> tmpList;
        barStream >> tmpList;
        qDebug() << barfilename << tmpList.size() << "bars";
        collectedBarList.append(tmpList);
    }

    int collectedSize = collectedBarList.size();
    QSet<int> invalidSet;
    for (int i = 0; i < collectedSize; i ++) {
        for (int j = i + 1; j < collectedSize; j++) {
            if (collectedBarList[i].time >= collectedBarList[j].time) {
                invalidSet.insert(j);
            }
        }
    }

    uint ktLastTime = 0;
    if (!barList.empty()) {
        ktLastTime = barList.last().time;
    }
    for (int i = 0; i < collectedSize; i++) {
        if (collectedBarList[i].time > ktLastTime && !invalidSet.contains(i)) {
            barList.append(collectedBarList[i]);
        }
    }

    int barListSize = barList.size();
    qDebug() << instrumentID << time_frame_str << " barListSize =" << barListSize;

    if (!collector_map.contains(instrumentID)) {
        qWarning() << "Warning! Missing collector for" << instrumentID << "!";
    }

    return &barList;
}

static QVariant getParam(const QByteArray &typeName, va_list &ap)
{
    QVariant ret;
    int typeId = QMetaType::type(typeName);
    switch (typeId) {
    case QMetaType::Int:
    {
        int value = va_arg(ap, int);
        ret.setValue(value);
    }
        break;
    case QMetaType::Double:
    {
        double value = va_arg(ap, double);
        ret.setValue(value);
    }
        break;
    default:
        break;
    }
    return ret;
}

AbstractIndicator* QuantTrader::registerIndicator(const QString &instrumentID, const QString &time_frame_str, QString indicator_name, ...)
{
    static const QMap<QString, const QMetaObject*> meta_object_map = {
        {"MA", &MA::staticMetaObject},
        {"ParabolicSAR", &ParabolicSAR::staticMetaObject},
        {"BollingerBand", &BollingerBand::staticMetaObject},
        {"AwesomeOscillator", &AwesomeOscillator::staticMetaObject},
        // Register more indicators here
    };

    if (instrumentID != nullptr && instrumentID.length() > 0) {
        currentInstrumentID = instrumentID;
    }
    if (time_frame_str != nullptr && time_frame_str.length() > 0) {
        currentTimeFrameStr = time_frame_str;
    }

    const QMetaObject * metaObject = meta_object_map.value(indicator_name, nullptr);
    if (metaObject == nullptr) {
        qCritical() << "Indicator" << indicator_name << "not exist!";
        return nullptr;
    }

    const int class_info_count = metaObject->classInfoCount();
    int parameter_number = 0;
    for (int i = 0; i < class_info_count; i++) {
        QMetaClassInfo classInfo = metaObject->classInfo(i);
        if (QString(classInfo.name()).compare("parameter_number", Qt::CaseInsensitive) == 0) {
            parameter_number = QString(classInfo.value()).toInt();
            qDebug() << parameter_number;
        }
    }

    va_list ap;
    va_start(ap, indicator_name);

    auto names = metaObject->constructor(0).parameterNames();
    auto types = metaObject->constructor(0).parameterTypes();
    QList<QVariant> params;
    for (int i = 0; i < parameter_number; i++) {
        params.append(getParam(types[i], ap));
    }
    va_end(ap);

    qDebug() << params;

    const auto pIndicators = indicator_map.values(currentInstrumentID);
    for (AbstractIndicator *pIndicator : pIndicators) {
        QObject *obj = dynamic_cast<QObject*>(pIndicator);
        if (indicator_name == obj->metaObject()->className()) {
            bool match = true;
            for (int i = 0; i < parameter_number; i++) {
                QVariant value = obj->property(names[i]);
                if (params[i] != value) {
                    match = false;
                }
            }
            if (match) {
                return pIndicator;
            }
        }
    }

    QVector<QGenericArgument> args;
    args.reserve(10);

    int int_param[10];
    int int_idx = 0;
    double double_param[10];
    int double_idx = 0;

#define Q_ENUM_ARG(type, data) QArgument<int >(type, data)

    for (int i = 0; i < parameter_number; i++) {
        int typeId = params[i].userType();
        switch (typeId) {
        case QMetaType::Int:
            int_param[int_idx] = params[i].toInt();
            args.append(Q_ENUM_ARG(types[i].constData(), int_param[int_idx]));
            int_idx ++;
            break;
        case QMetaType::Double:
            double_param[double_idx] = params[i].toDouble();
            args.append(Q_ARG(double, double_param[double_idx]));
            double_idx ++;
            break;
        default:
            args.append(QGenericArgument());
            break;
        }
    }
    args.append(Q_ARG(QObject*, this));

    QObject * obj =
    metaObject->newInstance(args.value(0), args.value(1), args.value(2),
                            args.value(3), args.value(4), args.value(5),
                            args.value(6), args.value(7), args.value(8), args.value(9));

    if (obj == 0) {
        qCritical() << "newInstance returns 0!";
        return nullptr;
    }

    auto* ret = dynamic_cast<AbstractIndicator*>(obj);

    indicator_map.insert(currentInstrumentID, ret);
    ret->setBarList(getBars(currentInstrumentID, currentTimeFrameStr), collector_map[currentInstrumentID]->getCurrentBar(currentTimeFrameStr));
    ret->update();

    return ret;
}

/*!
 * \brief QuantTrader::timesUp
 * 保存K线数据, 即刚结束的一段时间内的数据
 *
 * \param index 时间点序号
 */
void QuantTrader::timesUp(int index)
{
    const int size = saveBarTimePoints.size();
    if (index >= 0 && index < size) {
        for (const auto &collector : qAsConst(collectorsToSave[index])) {
            collector->saveBars();
        }
    } else {
        qFatal("Should never see this! index = %d", index);
    }
}

/*!
 * \brief QuantTrader::setTradingDay
 * 设定交易日期
 *
 * \param tradingDay 交易日(yyyyMMdd)
 */
void QuantTrader::setTradingDay(const QString &tradingDay)
{
    qDebug() << "Set Trading Day to" << tradingDay;

    for (auto * collector : qAsConst(collector_map)) {
        collector->setTradingDay(tradingDay);
    }
}

/*!
 * \brief QuantTrader::onMarketData
 * 处理市场数据, 如果有新的成交则计算相关策略
 * 统计相关策略给出的仓位, 如果与旧数值不同则发送给执行模块
 *
 * \param instrumentID 合约代码
 * \param time 时间
 * \param lastPrice 最新成交价
 * \param volume 成交量
 * \param askPrice1  卖一价
 * \param askVolume1 卖一量
 * \param bidPrice1  买一价
 * \param bidVolume1 买一量
 */
void QuantTrader::onMarketData(const QString& instrumentID, int time, double lastPrice, int volume,
                               double askPrice1, int askVolume1, double bidPrice1, int bidVolume1)
{
    Q_UNUSED(askPrice1)
    Q_UNUSED(askVolume1)
    Q_UNUSED(bidPrice1)
    Q_UNUSED(bidVolume1)

    BarCollector *collector = collector_map.value(instrumentID, nullptr);
    bool isNewTick = false;
    if (collector != nullptr) {
        isNewTick = collector->onMarketData(time, lastPrice, volume);
    }

    const auto strategyList = strategy_map.values(instrumentID);
    boost::optional<int> new_position_sum;
    for (auto *strategy : strategyList) {
        if (isNewTick) {    // 有新的成交
            strategy->onNewTick(time, lastPrice);
        }
        auto position = strategy->getPosition();
        if (position.is_initialized()) {
            if (new_position_sum.is_initialized()) {
                new_position_sum = new_position_sum.get() + position.get();
            } else {
                new_position_sum = position;
            }
        }
    }

    if (new_position_sum.is_initialized()) {
        if (position_map.contains(instrumentID) && position_map[instrumentID].is_initialized()) {
            if (position_map[instrumentID].get() != new_position_sum.get()) {
                position_map[instrumentID] = new_position_sum;
                pExecuter->cancelAllOrders(instrumentID);
                pExecuter->setPosition(instrumentID, new_position_sum.get());
                qDebug() << QTime(0, 0).addSecs(time).toString() << "New position for" << instrumentID << new_position_sum.get() << ", price =" << lastPrice;
            }
        } else {
            position_map[instrumentID] = new_position_sum;
            pExecuter->cancelAllOrders(instrumentID);
            pExecuter->setPosition(instrumentID, new_position_sum.get());
            qDebug() << QTime(0, 0).addSecs(time).toString() << "New position for" << instrumentID << new_position_sum.get() << ", price =" << lastPrice;
        }
    }
}

/*!
 * \brief QuantTrader::onNewBar
 * 储存新收集的K线数据并计算相关策略
 *
 * \param instrumentID 合约代码
 * \param time_frame 时间框架
 * \param bar 新的K线数据
 */
void QuantTrader::onNewBar(const QString &instrumentID, int time_frame, const Bar &bar)
{
    bars_map[instrumentID][time_frame].append(bar);
    const auto strategyList = strategy_map.values(instrumentID);
    for (auto *strategy : strategyList) {
        strategy->checkIfNewBar();
    }
}
