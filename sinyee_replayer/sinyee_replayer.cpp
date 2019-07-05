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

const QDateTime data_date_base = QDateTime::fromString("1988-01-01 00:00:00", "yyyy-MM-dd HH:mm:ss");
const QDateTime real_date_base = QDateTime::fromString("2008-01-01 00:00:00", "yyyy-MM-dd HH:mm:ss");
const int date_diff_int = data_date_base.secsTo(real_date_base);

SinYeeReplayer::SinYeeReplayer(const CONFIG_ITEM &config, QObject *parent) :
    CommonReplayer(parent)
{
    auto settings = getSettingsSmart(config.organization, "sinyee_replayer", this);

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

bool SinYeeReplayer::replayTo(int time)
{
    return CommonReplayer::replayTo(time - date_diff_int);
}
