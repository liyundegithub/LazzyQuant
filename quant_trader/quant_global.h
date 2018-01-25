#ifndef QUANT_GLOBAL_H
#define QUANT_GLOBAL_H

#include "indicator/ma.h"
#include "indicator/parabolicsar.h"
#include "indicator/bollinger_band.h"
#include "indicator/awesome_oscillator.h"
#include "strategy/DblMaPsar_strategy.h"
#include "strategy/bighit_strategy.h"

const QMap<QString, const QMetaObject*> indicatorMetaObjects = {
    {"MA", &MA::staticMetaObject},
    {"ParabolicSAR", &ParabolicSAR::staticMetaObject},
    {"BollingerBand", &BollingerBand::staticMetaObject},
    {"AwesomeOscillator", &AwesomeOscillator::staticMetaObject},
    // Register more indicators here
};

const QMap<QString, const QMetaObject*> strategyMetaObjects = {
    {"DblMaPsarStrategy", &DblMaPsarStrategy::staticMetaObject},
    {"BigHitStrategy", &BigHitStrategy::staticMetaObject},
    // Register more strategies here
};

#endif // QUANT_GLOBAL_H
