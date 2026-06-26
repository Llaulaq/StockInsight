#include <QApplication>
#include <QDebug>
#include "services/CsvParser.h"
#include "services/DataMerger.h"
#include "services/AnalyticsEngine.h"

void printLine(const QString &title) {
    qDebug() << "\n---" << title << "---";
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    printLine("Загрузка CSV");
    QVector<Product> products = CsvParser::parseProducts("data/products.csv");
    auto salesMap = CsvParser::parseSales("data/sales.csv");
    qDebug() << "Товаров:" << products.size() << "| Продаж:" << salesMap.size();
    
    // Этап 3: Объединение
    printLine("Объединение");
    DataMerger::merge(products, salesMap);
    
    // Этап 4: Аналитика
    printLine("Аналитика");
    Analytics a = AnalyticsEngine::calculate(products);
    
    qDebug() << "Прибыль:" << a.totalPotentialProfit;
    qDebug() << "Заморожено:" << a.frozenMoney;
    qDebug() << "Дефицит:" << a.deficitRiskCount;
    qDebug() << "Залежалых:" << a.staleCount;
    
    for (auto it = a.stockByCategory.begin(); it != a.stockByCategory.end(); ++it)
        qDebug() << " " << it.key() << ":" << it.value() << "шт.";
    
    for (const auto &p : a.staleProducts)
        qDebug() << " Залежалый:" << p.name << "| дней:" << p.daysInStock;
    
    return 0;
}