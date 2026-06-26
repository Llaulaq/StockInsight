#ifndef DATAMERGER_H
#define DATAMERGER_H

#include <QVector>
#include <QMap>
#include "../models/Product.h"

class DataMerger {
public:
    static void merge(QVector<Product> &products,
                      const QMap<QString, QVector<int>> &salesMap);
};

#endif x