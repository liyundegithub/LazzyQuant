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

/*!
 * \brief makeOptionID
 *
 * \param futureID 标的期货合约代码
 * \param dir Call or Put
 * \param exercisePrice 行权价
 * \return 期权合约代码
 */
QString makeOptionID(const QString &futureID, const OPTION_DIR dir, const int exercisePrice)
{
    QString middle = (dir == CALL_OPT) ? "-C-" : "-P-"; // FIXME SR705C6400
    QString optionID = futureID + middle + QString::number(exercisePrice);
    return optionID;
}


#define String const QString&

// 通过合约名获得文件的扩展名
QString getSuffix(String instrumentID) {
    const QString instrumentLowerCase = getInstrumentName(instrumentID).toLower();
    for (String instr : SQ) {
        if (instrumentLowerCase == instr) {
            return ".SQ";
        }
    }
    for (String instr : SY) {
        if (instrumentLowerCase == instr) {
            return ".SY";
        }
    }
    for (String instr : DL) {
        if (instrumentLowerCase == instr) {
            return ".DL";
        }
    }
    for (String instr : DY) {
        if (instrumentLowerCase == instr) {
            return ".DY";
        }
    }
    for (String instr : ZZ) {
        if (instrumentLowerCase == instr) {
            return ".ZZ";
        }
    }
    for (String instr : ZY) {
        if (instrumentLowerCase == instr) {
            return ".ZY";
        }
    }
    for (String instr : ZJ) {
        if (instrumentLowerCase == instr) {
            return ".ZJ";
        }
    }
    return ".notfound";
}
#undef String
