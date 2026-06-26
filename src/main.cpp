#include <QApplication>
#include <QDebug>
#include "services/CsvParser.h"
#include "services/DataMerger.h"
#include "services/AnalyticsEngine.h"
#include "services/JsonStorage.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QVector<Product> products = CsvParser::parseProducts("data/products.csv");
    auto salesMap = CsvParser::parseSales("data/sales.csv");
    qDebug() << "Товаров:" << products.size() << "| Продаж:" << salesMap.size();
    
    // Объединение
    DataMerger::merge(products, salesMap);
    
    // Аналитика
    Analytics a = AnalyticsEngine::calculate(products);
    
    qDebug() << "Прибыль:" << a.totalPotentialProfit;
    qDebug() << "Заморожено:" << a.frozenMoney;
    qDebug() << "Дефицит:" << a.deficitRiskCount;
    qDebug() << "Залежалых:" << a.staleCount;
    
    for (auto it = a.stockByCategory.begin(); it != a.stockByCategory.end(); ++it)
        qDebug() << " " << it.key() << ":" << it.value() << "шт.";
    
    for (const auto &p : a.staleProducts)
        qDebug() << " Залежалый:" << p.name << "| дней:" << p.daysInStock;

    // Сохранение JSON"
    JsonStorage::save(a, products, "output.json");
    
    return 0;
}