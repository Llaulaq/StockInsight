#include <QApplication>
#include <QDebug>
#include "services/CsvParser.h"
#include "services/DataMerger.h"
#include "services/AnalyticsEngine.h"
#include "services/JsonStorage.h"
#include "services/Logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Включаем логер
    Logger::instance().init("audit.log");
    
    // Загрузка CSV
    LOG_INFO("Загрузка CSV");
    QVector<Product> products = CsvParser::parseProducts("data/products.csv");
    auto salesMap = CsvParser::parseSales("data/sales.csv");
    qDebug() << "Товаров:" << products.size() << "| Продаж:" << salesMap.size();
    
    //Объединение
    LOG_INFO("Объединение данных");
    DataMerger::merge(products, salesMap);
    
    // Аналитика
    LOG_INFO("Расчёт метрик...");
    Analytics a = AnalyticsEngine::calculate(products);
    qDebug() << "Прибыль:" << a.totalPotentialProfit;
    qDebug() << "Заморожено:" << a.frozenMoney;
    qDebug() << "Дефицит:" << a.deficitRiskCount;
    qDebug() << "Залежалых:" << a.staleCount;
    
    // Сохранение JSON
    LOG_INFO("Сохранение JSON...");
    if (JsonStorage::save(a, products, "output.json"))
        LOG_INFO("JSON сохранён: output.json");
    else
        LOG_ERROR("Ошибка сохранения JSON!");
    
    LOG_INFO("Программа завершена");
    return 0;
}