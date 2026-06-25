#ifndef PRODUCT_H
#define PRODUCT_H

#include <QString>
#include <QDate>
#include <QVector>

struct Product {
    QString name;
    QString category;
    int quantity = 0;
    double purchasePrice = 0.0;
    double salePrice = 0.0;
    QDate deliveryDate;

    // Расчётные поля
    int daysInStock = 0;
    QVector<int> monthlySales;
    double avgSalesPerMonth = 0.0;
    double totalProfit = 0.0;
    bool isDeficit = false;
    bool isStale = false;
};

#endif // PRODUCT_H