#include <QStringList>

#include "utility.h"

/*!
 * \brief getInstrumentName
 * 从期货或期权合约代码里提取交易品种名
 * 比如 cu1703 --> cu, i1705 --> i, CF705 --> CF
 *      m1707-C-2700 --> m
 *
 * \param instrumentID 期货/期权合约代码
 * \return 交易品种名
 */
QString getInstrumentName(const QString &instrumentID)
{
    const int len = instrumentID.length();
    int i = 0;
    for (; i < len; i++) {
        if (instrumentID[i].isDigit()) {
            break;
        }
    }
    return instrumentID.left(i);
}

/*!
 * \brief getFutureIDFromOptionID
 * 从期权合约代码里提取对应标的期货合约代码
 * 比如 m1707-C-2700 --> m1707, SR705C6400 --> SR705
 *
 * \param instrumentID 期权合约代码
 * \return 标的期货合约代码
 */
QString getFutureIDFromOptionID(const QString &instrumentID)
{
    const int len = instrumentID.length();
    int i = 0;
    for (; i < len; i++) {
        if (instrumentID[i].isDigit()) {
            break;
        }
    }
    for (; i < len; i++) {
        if (!instrumentID[i].isDigit()) {
            break;
        }
    }
    return instrumentID.left(i);
}
