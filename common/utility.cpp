#include <QString>

#include "utility.h"

/*!
 * \brief getInstrumentName
 * 从合约代码里提取交易品种名(就是去掉交割月份)
 * 比如 cu1703 --> cu, i1705 --> i, CF705 --> CF
 *
 * \param instrumentID 合约代码
 * \return 交易品种名
 */
QString getInstrumentName(const QString &instrumentID) {
    const int len = instrumentID.length();
    int i = 0;
    for (; i < len; i++) {
        if (instrumentID[i].isDigit()) {
            break;
        }
    }
    return instrumentID.left(i);
}
