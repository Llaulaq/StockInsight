#include "test_csv_parser.h"
#include <QTemporaryFile>
#include <QTextStream>

void TestCsvParser::testParseProducts() {
    QTemporaryFile file;
    file.open();
    QTextStream out(&file);
    out << "Товар;Категория;Количество;Цена_закупки;Цена_продажи;Дата_поставки\n";
    out << "Мышь;Периферия;45;1200;2500;2026-05-15\n";
    out << "Клавиатура;Периферия;12;3500;6500;2026-05-01\n";
    out.flush();
    
    QVector<Product> products = CsvParser::parseProducts(file.fileName());
    QCOMPARE(products.size(), 2);
    QCOMPARE(products[0].name, "Мышь");
    QCOMPARE(products[0].quantity, 45);
    QCOMPARE(products[1].name, "Клавиатура");
}

void TestCsvParser::testParseSales() {
    QTemporaryFile file;
    file.open();
    QTextStream out(&file);
    out << "Товар;Янв;Фев;Мар;Апр;Май;Июнь\n";
    out << "Мышь;10;12;8;15;20;18\n";
    out.flush();
    
    auto sales = CsvParser::parseSales(file.fileName());
    QCOMPARE(sales.size(), 1);
    QVERIFY(sales.contains("Мышь"));
    QCOMPARE(sales["Мышь"].size(), 6);
    QCOMPARE(sales["Мышь"][0], 10);
}

void TestCsvParser::testEmptyFile() {
    QTemporaryFile file;
    file.open();
    file.close();
    
    QVector<Product> products = CsvParser::parseProducts(file.fileName());
    QVERIFY(products.isEmpty());
}