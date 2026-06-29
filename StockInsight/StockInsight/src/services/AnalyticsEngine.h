#ifndef ANALYTICSENGINE_H
#define ANALYTICSENGINE_H

#include "../models/Analytics.h"
#include <QDate>

class AnalyticsEngine {
public:
    static Analytics calculate(QVector<Product> &products,
                               const QDate &currentDate = QDate::currentDate());
};

#endif 