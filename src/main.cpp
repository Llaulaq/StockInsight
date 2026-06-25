#include <QString>
#include "models/Product.h"
#include <QApplication>
#include <QDebug>
#include "services/CsvParser.h"   


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== Этап 2: Тест парсера CSV ===";
    
    QString productPath = "/home/stone/programming/StockInsight/data/products.csv";
    QString salesPath = "/home/stone/programming/StockInsight/data/sales.csv";
    
    QVector<Product> products = CsvParser::parseProducts(productPath);
    qDebug() << "Товаров:" << products.size();
    for (const auto &p : products) {
        qDebug() << " " << p.name << "|" << p.category << "|" << p.quantity << "шт.";
    }
    
    auto salesMap = CsvParser::parseSales(salesPath);
    qDebug() << "Продаж:" << salesMap.size();
    for (auto it = salesMap.begin(); it != salesMap.end(); ++it) {
        qDebug() << " " << it.key() << ":" << it.value();
    }
    
    return 0;
}