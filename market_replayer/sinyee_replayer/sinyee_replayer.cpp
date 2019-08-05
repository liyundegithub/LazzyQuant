#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QDataStream>
#include <QPair>

#include "config_struct.h"
#include "common_utility.h"
#include "sinyee_tick.h"
#include "sinyee_bar.h"
#include "sinyee_replayer.h"

SinYeeReplayer::SinYeeReplayer(const CONFIG_ITEM &config, QObject *parent) :
    CommonReplayer(parent)
{
    auto settings = getSettingsSmart(config.organization, "sinyee_replayer", this);
    sinYeeDataPath = settings->value("SinYeeDataPath").toString();
    this->replayList = getSettingItemList(settings.get(), "ReplayList");
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

    TimeMapper mapTime;
    mapTime.setTradingDay(date);
    for (const auto &contractName : contractNames) {
        QSet<int> minutes;  // 用于检查某个时间是否是在交易时段内.

        const auto offsetNums = contractOffsetNum.values(contractName);
        if (!offsetNums.isEmpty()) {
            auto offset = offsetNums[0].first;
            auto num = offsetNums[0].second;
            barStream.startTransaction();
            barStream.skipRawData(offset);
            auto oneMinuteBars = SinYeeBar::readBars(barStream, num);
            barStream.rollbackTransaction();

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

            for (const auto &bar : qAsConst(oneMinuteBars)) {
                minutes << bar.time / 60;
            }
        }

        qint32 num;
        tickStream >> num;
        Q_ASSERT(num >= 0);
        qInfo() << num << "items for" << contractName;

        const auto tickList = SinYeeTick::readTicks(tickStream, num);
        if (!contractName.endsWith("99")) {
            QString normalizedContractName = contractName;
            if (instrument.startsWith("SQ") || instrument.startsWith("DL")) {
                // 上期所和大商所的合约代码为小写.
                normalizedContractName.replace(0, 2, contractName.left(2).toLower());
            }
            int sumVol = 0;
            for (const auto &sinYeeTick : tickList) {
                if (minutes.contains(sinYeeTick.time / 60)) {
                    sumVol += sinYeeTick.volume;
                    CommonTick commonTick = {0,
                                             sinYeeTick.price,
                                             sinYeeTick.askPrice,
                                             sinYeeTick.bidPrice,
                                             sumVol,
                                             (int)sinYeeTick.askVolume,
                                             (int)sinYeeTick.bidVolume};
                    int hhmmssTime = sinYeeTick.time % 86400;
                    auto hour = hhmmssTime / 3600;
                    if (hour < 8) {
                        hhmmssTime -= (4 * 3600);
                        if (hhmmssTime < 0) {
                            hhmmssTime += 86400;
                        }
                    }
                    commonTick.setTimeStamp(mapTime(hhmmssTime), sinYeeTick.msec);
                    tickPairList << qMakePair(normalizedContractName, commonTick);
                }
            }
        }
    }
    tickFile.close();
}
