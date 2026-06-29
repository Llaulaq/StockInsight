#ifndef JSONSTORAGE_H
#define JSONSTORAGE_H

#include <QString>
#include "../models/Analytics.h"

class JsonStorage {
public:
    static bool save(const Analytics &analytics,
                     const QVector<Product> &products,
                     const QString &filePath);
};

#endif