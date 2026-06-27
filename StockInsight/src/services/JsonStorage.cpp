#include "JsonStorage.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>

bool JsonStorage::save(const Analytics &a,
                       const QVector<Product> &products,
                       const QString &filePath) {
    QJsonObject root;

    // Блок summary
    QJsonObject summary;
    summary["total_potential_profit"] = a.totalPotentialProfit;
    summary["frozen_money"] = a.frozenMoney;
    summary["deficit_risk_count"] = a.deficitRiskCount;
    summary["stale_count"] = a.staleCount;
    root["summary"] = summary;

    // Массив products
    QJsonArray jsonProducts;
    for (const auto &p : products) {
        QJsonObject obj;
        obj["name"] = p.name;
        obj["category"] = p.category;
        obj["quantity"] = p.quantity;
        obj["profit_per_unit"] = p.salePrice - p.purchasePrice;
        obj["total_profit"] = p.totalProfit;
        obj["days_in_stock"] = p.daysInStock;
        obj["is_stale"] = p.isStale;
        obj["is_deficit"] = p.isDeficit;
        obj["avg_sales_per_month"] = p.avgSalesPerMonth;

        QJsonArray salesArr;
        for (int s : p.monthlySales)
            salesArr.append(s);
        obj["monthly_sales"] = salesArr;

        jsonProducts.append(obj);
    }
    root["products"] = jsonProducts;

    // Блок charts_data
    QJsonObject charts;

    QJsonObject stockCat;
    for (auto it = a.stockByCategory.begin(); it != a.stockByCategory.end(); ++it)
        stockCat[it.key()] = it.value();
    charts["stock_by_category"] = stockCat;

    QJsonObject profitCat;
    for (auto it = a.profitByCategory.begin(); it != a.profitByCategory.end(); ++it)
        profitCat[it.key()] = it.value();
    charts["profit_by_category"] = profitCat;

    QJsonObject salesMonth;
    for (auto it = a.salesByMonth.begin(); it != a.salesByMonth.end(); ++it) {
        QJsonArray arr;
        for (int s : it.value())
            arr.append(s);
        salesMonth[it.key()] = arr;
    }
    charts["sales_by_month"] = salesMonth;

    root["charts_data"] = charts;

    // Запись в файл
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Не удалось сохранить JSON:" << filePath;
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "JSON сохранён:" << filePath;
    return true;
}