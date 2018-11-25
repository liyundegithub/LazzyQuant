#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QDataStream>
#include <QPair>

#include "config_struct.h"
#include "common_utility.h"
#include "sinyee_bar.h"
#include "sinyee_replayer.h"

const QDateTime data_date_base = QDateTime::fromString("1988-01-01 00:00:00", "yyyy-MM-dd HH:mm:ss");
const QDateTime real_date_base = QDateTime::fromString("2008-01-01 00:00:00", "yyyy-MM-dd HH:mm:ss");
const int date_diff_int = data_date_base.secsTo(real_date_base);

SinYeeReplayer::SinYeeReplayer(const CONFIG_ITEM &config, QObject *parent) :
    QObject(parent)
{
    auto settings = getSettingsSmart(config.organization, config.name, this);

    sinYeeDataPath = settings->value("SinYeeDataPath").toString();

    settings->beginGroup("ReplayList");
    const auto replayListTmp = settings->childKeys();
    for (const auto &key : replayListTmp) {
        if (settings->value(key).toBool()) {
            replayList.append(key);
        }
    }
    settings->endGroup();
    replayList.removeDuplicates();
}

void SinYeeReplayer::appendTicksToList(const QString &date, const QString &instrument)
{
    QString tickFilePath = sinYeeDataPath + "/" + instrument + "/" + instrument + "_" + date + ".tick";
    QFile tickFile(tickFilePath);
    if (!tickFile.open(QFile::ReadOnly)) {
        qCritical() << "Open file:" << tickFilePath << "failed!";
        return;
    }
    QDataStream tickStream(&tickFile);
    tickStream.setByteOrder(QDataStream::LittleEndian);
    tickStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    int skipped = tickStream.skipRawData(6);
    Q_ASSERT(skipped == 6);

    const QStringList contractNames = SinYeeTick::getAvailableContracts(tickStream);
    qDebug() << contractNames;

    QString barFilePath = sinYeeDataPath + "/" + instrument + "/" + instrument + "_" + date + ".bar";
    QFile barFile(barFilePath);
    if (!barFile.open(QFile::ReadOnly)) {
        qCritical() << "Open file:" << barFilePath << "failed!";
        return;
    }
    QDataStream barStream(&barFile);
    barStream.setByteOrder(QDataStream::LittleEndian);
    barStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    barStream.startTransaction();
    skipped = barStream.skipRawData(4);
    Q_ASSERT(skipped == 4);
    const auto contractOffsetNum = SinYeeBar::getAvailableContracts(barStream);
    barStream.rollbackTransaction();

    for (const auto &contractName : contractNames) {
        QList<SinYeeBar> oneMinuteBars;     // 用于检查某个时间是否是在交易时段内.
        auto isValidTime = [&oneMinuteBars](int tickTime) -> bool {
            auto tickMinute = tickTime / 60 * 60;
            for (const auto &bar : qAsConst(oneMinuteBars)) {
                if (bar.time == tickMinute) {
                    return true;
                }
            }
            return false;
        };

        const auto offsetNums = contractOffsetNum.values(contractName);
        if (!offsetNums.isEmpty()) {
            auto offset = offsetNums[0].first;
            auto num = offsetNums[0].second;
            barStream.startTransaction();
            barStream.skipRawData(offset);
            oneMinuteBars = SinYeeBar::readBars(barStream, num);
            barStream.rollbackTransaction();
        }

        // 由于数据源的质量问题，可能需要把夜盘最后一分钟不属于交易时段的Bar去除.
        int i = 0;
        int size = oneMinuteBars.size();
        for (; i < size; i++) {
            QTime barTime = QTime(0, 0).addSecs(oneMinuteBars[i].time);
            if (barTime == QTime(9, 0)) {
                break;
            }
        }
        if (i > 0 && i != size) {
            if (i % 5 == 1) {
                oneMinuteBars.removeAt(i - 1);
            }
        }

        qint32 num;
        tickStream >> num;
        Q_ASSERT(num >= 0);
        qInfo() << num << "items for" << contractName;

        const auto tickList = SinYeeTick::readTicks(tickStream, num);
        if (!contractName.endsWith("99")) {
            for (const auto &tick : tickList) {
                if (isValidTime(tick.time)) {
                    tickPairList << qMakePair(contractName, tick);
                }
            }
        }
    }
    tickFile.close();
}

void SinYeeReplayer::sortTickPairList()
{
    std::stable_sort(tickPairList.begin(), tickPairList.end(), [](const auto &item1, const auto &item2) -> bool {
        if (item1.second.time == item2.second.time) {
            return item1.second.msec < item2.second.msec;
        } else {
            return item1.second.time < item2.second.time;
        }
    });
    tickCnt = tickPairList.size();
    replayIdx = 0;
}

/*!
 * \brief SinYeeReplayer::startReplay
 * 复盘指定日期的行情, 复盘合约由配置文件中ReplayList指定
 *
 * \param date 复盘日期
 */
void SinYeeReplayer::startReplay(const QString &date)
{
    startReplay(date, replayList);
}

/*!
 * \brief SinYeeReplayer::startReplay
 * 复盘指定日期的单个合约行情, 复盘合约由参数instrument指定
 *
 * \param date 复盘日期
 * \param instrument 复盘合约
 */
void SinYeeReplayer::startReplay(const QString &date, const QString &instrument)
{
    startReplay(date, QStringList() << instrument);
}

/*!
 * \brief SinYeeReplayer::startReplay
 * 复盘指定日期的多个合约行情, 复盘合约列表由参数instruments指定
 *
 * \param date 复盘日期
 * \param instruments 复盘合约列表
 */
void SinYeeReplayer::startReplay(const QString &date, const QStringList &instruments)
{
    prepareReplay(date, instruments);
    replayTo(INT_MAX);
}

bool SinYeeReplayer::prepareReplay(const QString &date)
{
    return prepareReplay(date, replayList);
}

bool SinYeeReplayer::prepareReplay(const QString &date, const QStringList &instruments)
{
    replayDate = date;
    tickPairList.clear();
    sumVol.clear();
    for (const auto &instrument : instruments) {
        appendTicksToList(date, instrument);
    }
    sortTickPairList();
    if (tickCnt > 0) {
        emit tradingDayChanged(date);
        mapTime.setTradingDay(date);
    }
    return tickCnt > 0;
}

bool SinYeeReplayer::replayTo(int time)
{
    time -= date_diff_int;
    bool ret = false;
    if (tickCnt > 0) {
        for (; replayIdx < tickCnt; replayIdx++) {
            const auto &item = tickPairList[replayIdx];
            const auto &tick = item.second;
            if (time >= tick.time) {
                int emitTime = tick.time % 86400;
                sumVol[item.first] += tick.volume;
                auto hour = emitTime / 3600;
                if (hour < 8) {
                    emitTime -= (4 * 3600);
                    if (emitTime < 0) {
                        emitTime += 86400;
                    }
                }
                emit newMarketData(item.first,
                                   mapTime(emitTime),
                                   tick.price,
                                   sumVol[item.first],
                                   tick.askPrice,
                                   tick.askVolume,
                                   tick.bidPrice,
                                   tick.bidVolume);
                ret = true;
            } else {
                break;
            }
        }
        if (replayIdx >= tickCnt) {
            emit endOfReplay(replayDate);
            tickCnt = 0;
        }
    }
    return ret;
}
