#ifndef TEST_CSV_PARSER_H
#define TEST_CSV_PARSER_H

#include <QtTest>
#include "../src/services/CsvParser.h"

class TestCsvParser : public QObject {
    Q_OBJECT

private slots:
    void testParseProducts();
    void testParseSales();
    void testEmptyFile();
};

#endif