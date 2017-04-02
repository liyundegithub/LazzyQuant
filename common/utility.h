#ifndef UTILITY_H
#define UTILITY_H

#include <QString>

enum OPTION_TYPE {
    CALL_OPT,
    PUT_OPT,
};

QString getCode(const QString &instrumentID);
bool parseOptionID(const QString &optionID, QString &futureID, OPTION_TYPE &type, int &exercisePrice);
QString makeOptionID(const QString &futureID, const OPTION_TYPE type, const int exercisePrice);

// 上海期货交易所                                   燃油, 线材
const QString SQ[] = {"fu", "wr"};
// 上海期货交易所 (夜盘)              铜,   铝,   锌,   铅,   镍,   锡,   金,   银,螺纹钢,热轧卷板,沥青,天然橡胶
const QString SY[] = {"cu", "al", "zn", "pb", "ni", "sn", "au", "ag", "rb", "hc", "bu", "ru"};
// 大连商品交易所                                  玉米, 玉米淀粉, 纤维板,  胶合板, 鸡蛋, 线型低密度聚乙烯, 聚氯乙烯, 聚丙烯
const QString DL[] = {"c",  "cs", "fb", "bb", "jd", "l",  "v",  "pp"};
// 大连商品交易所  (夜盘)          黄大豆1号, 黄大豆2号, 豆粕, 大豆原油, 棕榈油, 冶金焦炭, 焦煤, 铁矿石
const QString DY[] = {"a",  "b",  "m",  "y",  "p",  "j",  "jm", "i"};
// 郑州商品交易所
const QString ZZ[] = {"jr", "lr", "pm", "ri", "rs", "sf", "sm", "wh"};
// 郑州商品交易所 (夜盘)
const QString ZY[] = {"cf", "fg", "ma", "oi", "rm", "sr", "ta", "zc", "tc"};	// zc原来为tc
// 中金所
const QString ZJ[] = {"ic", "if", "ih", "t",  "tf"};

QString getSuffix(const QString &instrumentID);

#endif // UTILITY_H
