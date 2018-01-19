#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QDataStream>
#include <QPair>

#include "config_struct.h"
#include "common_utility.h"
#include "sinyee_tick.h"
#include "sinyee_replayer.h"
#include "sinyee_replayer_adaptor.h"

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

    new Sinyee_replayerAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(config.dbusObject, this);
    dbus.registerService(config.dbusService);
}

QStringList getAvailableContracts(QDataStream& tickStream)
{
    qint16 num;
    tickStream >> num;
    Q_ASSERT(num >= 0);

    char buf[32];
    QStringList contracts;

    for (int i = 0; i < num; i ++) {
        qint8 strlen;
        tickStream >> strlen;
        Q_ASSERT(strlen > 0);

        memset(buf, 0, 32);
        tickStream.readRawData(buf, strlen);
        QString contract(buf);
        contracts << contract;

        tickStream.skipRawData(29 - strlen);
    }

    return contracts;
}

QList<SinYeeTick> readTicks(QDataStream& tickStream, int num)
{
    QList<SinYeeTick> tickList;
    for (int i = 0; i < num; i++) {
        SinYeeTick tick;
        tickStream >> tick;
        tickList << tick;
    }
    return tickList;
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
    QList<QPair<QString, SinYeeTick>> tickPairList;

    for (const auto &instrument : instruments) {
        QString tickFilePath = sinYeeDataPath + "/" + instrument + "/" + instrument + "_" + date + ".tick";
        QFile tickFile(tickFilePath);
        if (!tickFile.open(QFile::ReadOnly)) {
            qCritical() << "Open file:" << tickFilePath << "failed!";
            continue;
        }
        QDataStream tickStream(&tickFile);
        tickStream.setByteOrder(QDataStream::LittleEndian);
        tickStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        int skipped = tickStream.skipRawData(6);
        Q_ASSERT(skipped == 6);

        const QStringList contractNames = getAvailableContracts(tickStream);
        qDebug() << contractNames;
        for (const auto &contractName : contractNames) {
            qint32 num;
            tickStream >> num;
            Q_ASSERT(num >= 0);
            qInfo() << num << "items for" << contractName;

            const auto tickList = readTicks(tickStream, num);
            if (!contractName.endsWith("99")) {
                for (const auto &item : tickList) {
                    tickPairList << qMakePair(contractName, item);
                }
            }
        }
        tickFile.close();
    }

    std::stable_sort(tickPairList.begin(), tickPairList.end(), [](const auto &item1, const auto &item2) -> bool {
        if (item1.second.time == item2.second.time) {
            return item1.second.msec < item2.second.msec;
        } else {
            return item1.second.time < item2.second.time;
        }
    });

    if (!tickPairList.empty()) {
        emit tradingDayChanged(date);
        QMap<QString, int> sumVol;
        for (const auto &item : qAsConst(tickPairList)) {
            const auto &tick = item.second;
            const int emitTime = tick.time % 86400;
            sumVol[item.first] += tick.volume;
            emit newMarketData(item.first,
                               emitTime,
                               tick.price,
                               sumVol[item.first],
                               tick.askPrice,
                               tick.askVolume,
                               tick.bidPrice,
                               tick.bidVolume);
        }
        emit endOfReplay(date);
    }
}
