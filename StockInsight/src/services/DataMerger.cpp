#include "DataMerger.h"
#include <QDebug>

void DataMerger::merge(QVector<Product> &products,
                       const QMap<QString, QVector<int>> &salesMap) {
    int found = 0;
    int notFound = 0;

    for (auto &product : products) {
        if (salesMap.contains(product.name)) {
            product.monthlySales = salesMap[product.name];
            found++;
        } else {
            product.monthlySales = QVector<int>(6, 0);
            notFound++;
            qDebug() << "Не найдены продажи для:" << product.name;
        }
    }

    qDebug() << "Объединено:" << found << "товаров, не найдено:" << notFound;
}