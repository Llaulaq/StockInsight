#include "test_analytics_engine.h"

void TestAnalyticsEngine::testProfit() {
    QVector<Product> products;
    Product p;
    p.name = "Товар";
    p.category = "Кат";
    p.quantity = 10;
    p.purchasePrice = 100;
    p.salePrice = 200;
    p.deliveryDate = QDate(2026, 1, 1);
    p.monthlySales = {5, 5, 5, 5, 5, 5};
    products.append(p);
    
    Analytics a = AnalyticsEngine::calculate(products, QDate(2026, 7, 1));
    QCOMPARE(a.totalPotentialProfit, 1000.0); // (200-100)*10
}

void TestAnalyticsEngine::testDeficit() {
    QVector<Product> products;
    Product p;
    p.name = "Товар";
    p.category = "Кат";
    p.quantity = 5;                        // мало
    p.purchasePrice = 10;
    p.salePrice = 20;
    p.deliveryDate = QDate(2026, 6, 1);
    p.monthlySales = {10, 10, 10, 10, 10, 10}; // avg = 10
    products.append(p);
    
    Analytics a = AnalyticsEngine::calculate(products, QDate(2026, 7, 1));
    QCOMPARE(a.deficitRiskCount, 1); // 5 < 10*2
}

void TestAnalyticsEngine::testStale() {
    QVector<Product> products;
    Product p;
    p.name = "Старый";
    p.category = "Кат";
    p.quantity = 10;
    p.purchasePrice = 100;
    p.salePrice = 200;
    p.deliveryDate = QDate(2025, 1, 1);    // давно
    p.monthlySales = {5, 5, 5, 5, 5, 5};
    products.append(p);
    
    Analytics a = AnalyticsEngine::calculate(products, QDate(2026, 7, 1));
    QCOMPARE(a.staleCount, 1); // days > 90
}