#ifndef COMMON_UTILITY_H
#define COMMON_UTILITY_H

#include <memory>

#include <QMetaEnum>
#include <QString>
#include <QDateTime>

#define DATE_TIME (QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz")))
#define INVALID_DATE_STRING QStringLiteral("00000000")

#define TM1 \
    QTime t1 = QTime::currentTime();
#define TM2 \
    qWarning() << t1.msecsTo(QTime::currentTime()) << "ms";

static inline bool isTimeCloseEnouogh(uint time1, uint time2, uint diff)
{
    // 两个无符号数不能用qAbs(time1 - time2) < diff;
    if (time1 > time2) {
        return (time1 - time2) < diff;
    } else {
        return (time2 - time1) < diff;
    }
}

static inline bool isTimeCloseEnouogh(uint time1, uint time2, uint time3, uint diff)
{
    return isTimeCloseEnouogh(time1, time2, diff) && isTimeCloseEnouogh(time2, time3, diff) && isTimeCloseEnouogh(time1, time3, diff);
}

enum OPTION_TYPE {
    CALL_OPT,
    PUT_OPT,
};

QString getCode(const QString &instrumentID);
bool parseOptionID(const QString &optionID, QString &futureID, OPTION_TYPE &type, int &exercisePrice);
QString makeOptionID(const QString &futureID, const OPTION_TYPE type, const int exercisePrice);
bool isOption(const QString &instrumentID);

class QSettings;
class QObject;
std::unique_ptr<QSettings> getSettingsSmart(const QString &organization, const QString &name, QObject *parent = nullptr);

template<typename EnumType>
QList<int> enumValueToList(int value)
{
    QList<int> valueList;
    auto mobj = QMetaEnum::fromType<EnumType>();
    int count = mobj.keyCount();
    for (int i = 0; i < count; i++) {
        int k = mobj.value(i);
        if ((k != 0) && ((value & k) == k)) {
            valueList.append(k);
        }
    }
    return valueList;
}

static inline quint8 charToDigit(const char ten, const char one)
{
    return quint8(10 * (ten - '0') + one - '0');
}

// Convert string with format hh:mm:ss into seconds
static inline int hhmmssToSec(const char *hhmmss)
{
    quint8 hour, minute, second;
    hour   = charToDigit(hhmmss[0], hhmmss[1]);
    minute = charToDigit(hhmmss[3], hhmmss[4]);
    second = charToDigit(hhmmss[6], hhmmss[7]);
    return hour * 3600 + minute * 60 + second;
}

template<class T>
static inline bool isWithinRange(const T &t, const T &rangeStart, const T &rangeEnd)
{
    if (rangeStart < rangeEnd) {
        return rangeStart <= t && t <= rangeEnd;
    } else {
        return rangeStart <= t || t <= rangeEnd;
    }
}

template<class T>
static inline bool isWithinRangeExcludeEnd(const T &t, const T &rangeStart, const T &rangeEnd)
{
    if (rangeStart < rangeEnd) {
        return rangeStart <= t && t < rangeEnd;
    } else {
        return rangeStart <= t || t < rangeEnd;
    }
}

// 三种类型的订单 (0:普通限价单, 1:Fill and Kill, 2:Fill or Kill)
#define LIMIT_ORDER   0
#define FAK_ORDER     1
#define FOK_ORDER     2


// 上海期货交易所                                   燃油, 线材.
const QString SQ[] = {"fu", "wr"};
// 上海期货交易所 (夜盘)              铜,   铝,   锌,   铅,   镍,   锡,   金,   银,螺纹钢,热轧卷板,沥青,天然橡胶.
const QString SY[] = {"cu", "al", "zn", "pb", "ni", "sn", "au", "ag", "rb", "hc", "bu", "ru"};
// 大连商品交易所                                  玉米, 玉米淀粉, 纤维板,  胶合板, 鸡蛋, 线型低密度聚乙烯, 聚氯乙烯, 聚丙烯.
const QString DL[] = {"c",  "cs", "fb", "bb", "jd", "l",  "v",  "pp"};
// 大连商品交易所  (夜盘)          黄大豆1号, 黄大豆2号, 豆粕, 大豆原油, 棕榈油, 冶金焦炭, 焦煤, 铁矿石.
const QString DY[] = {"a",  "b",  "m",  "y",  "p",  "j",  "jm", "i"};
// 郑州商品交易所.
const QString ZZ[] = {"jr", "lr", "pm", "ri", "rs", "sf", "sm", "wh", "ap"};
// 郑州商品交易所 (夜盘)
const QString ZY[] = {"cf", "cy", "fg", "ma", "oi", "rm", "sr", "ta", "zc", "tc"};	// zc原来为tc
// 中金所.
const QString ZJ[] = {"ic", "if", "ih", "t",  "tf", "ts"};

QString getSuffix(const QString &instrumentID);

#endif // COMMON_UTILITY_H
