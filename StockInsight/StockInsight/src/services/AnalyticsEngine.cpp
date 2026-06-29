#include "AnalyticsEngine.h"
#include <algorithm>
#include <QDebug>

Analytics AnalyticsEngine::calculate(QVector<Product> &products,
                                     const QDate &currentDate) {
    Analytics a;

    for (auto &p : products) {
        // Дни на складе
        p.daysInStock = p.deliveryDate.daysTo(currentDate);
        if (p.daysInStock < 0) p.daysInStock = 0;

        // Средние продажи в месяц
        double sum = 0;
        for (int s : p.monthlySales) sum += s;
        p.avgSalesPerMonth = p.monthlySales.isEmpty() ? 0 : sum / p.monthlySales.size();

        // Прибыль
        p.totalProfit = (p.salePrice - p.purchasePrice) * p.quantity;
        a.totalPotentialProfit += p.totalProfit;

        // Дефицит
        p.isDeficit = (p.quantity < p.avgSalesPerMonth * 2);
        if (p.isDeficit) a.deficitRiskCount++;

        // Залежалость
        p.isStale = (p.daysInStock > 90) || (p.avgSalesPerMonth < 1.0);
        if (p.isStale) {
            a.staleCount++;
            a.frozenMoney += p.quantity * p.purchasePrice;
            a.staleProducts.append(p);
        }

        // Агрегация по категориям
        a.stockByCategory[p.category] += p.quantity;
        a.profitByCategory[p.category] += p.totalProfit;

        // Данные для графика продаж
        a.salesByMonth[p.name] = p.monthlySales;
    }

    // Топ-5 залежалых
    std::sort(a.staleProducts.begin(), a.staleProducts.end(),
              [](const Product &x, const Product &y) {
                  return x.daysInStock > y.daysInStock;
              });
    if (a.staleProducts.size() > 5) a.staleProducts.resize(5);


    qDebug() << "Прибыль" << a.totalPotentialProfit;
    qDebug() << "Заморожено" << a.frozenMoney;
    qDebug() << "Дефицит" << a.deficitRiskCount;
    qDebug() << "Залежалых" << a.staleCount;

    return a;
}