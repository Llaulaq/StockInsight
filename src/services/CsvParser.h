#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <QVector>
#include <QMap>
#include <QString>
#include "../models/Product.h"    
class CsvParser {
public:
    static QVector<Product> parseProducts(const QString &filePath);
    static QMap<QString, QVector<int>> parseSales(const QString &filePath);
};

#endif 