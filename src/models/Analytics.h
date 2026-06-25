#ifndef ANALYTICS_H
#define ANALYTICS_H

#include "Product.h"
#include <QMap>
#include <QVector>

struct Analytics {
    double totalPotentialProfit = 0.0;
    double frozenMoney = 0.0;
    int deficitRiskCount = 0;
    int staleCount = 0;

    QMap<QString, int> stockByCategory;
    QMap<QString, double> profitByCategory;
    QMap<QString, QVector<int>> salesByMonth;
    QVector<Product> staleProducts;
};

#endif // ANALYTICS_H