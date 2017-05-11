#include <QSet>

#include "option_index.h"

OptionIndex::OptionIndex(const QMultiMap<QString, int> &underlyingKMap) :
    underlyingKMap(underlyingKMap)
{
    underlyingList = underlyingKMap.uniqueKeys();
    underlyingNum = underlyingList.count();
    for (int i = 0; i < underlyingNum; i++) {
        underlyingToIdxMap.insert(underlyingList[i], i);
    }

    kList = underlyingKMap.values().toSet().toList();
    std::sort(kList.begin(), kList.end());
    kNum = kList.count();
    pK = (int *) malloc(kNum * sizeof(int));
    for (int i = 0; i < kNum; i++) {
        pK[i] = kList[i];
        kToIdxMap.insert(kList[i], i);
    }

    for (int i = 0; i < underlyingNum; i++) {
        auto underlyingKList = underlyingKMap.values(underlyingList[i]);
        std::sort(underlyingKList.begin(), underlyingKList.end());
        underlyingIdxKList << underlyingKList;

        QList<int> underlyingKIdxList;
        for (const auto k : qAsConst(underlyingKList)) {
            underlyingKIdxList << kToIdxMap[k];
        }
        std::sort(underlyingKIdxList.begin(), underlyingKIdxList.end());
        underlyingIdxKIdxList << underlyingKIdxList;
    }
}

OptionIndex::~OptionIndex()
{
    free(pK);
}

bool OptionIndex::parseOptionIdx(const QString &optionID, int &underlyingIdx, OPTION_TYPE &type, int &kIdx) const
{
    QString underlyingID;
    int K;

    if (parseOptionID(optionID, underlyingID, type, K)) {
        underlyingIdx = getIdxByUnderlyingID(underlyingID);
        kIdx = getIdxByK(K);
        if (underlyingIdx != -1 && kIdx != -1) {
            return true;
        }
    }
    return false;
}

QString OptionIndex::makeOptionByIdx(int underlyingIdx, OPTION_TYPE type, int kIdx) const
{
    QString underlyingID = getUnderlyingIDByIdx(underlyingIdx);
    int K = getKByIdx(kIdx);
    return makeOptionID(underlyingID, type, K);
}
