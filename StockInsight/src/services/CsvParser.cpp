#include "CsvParser.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStringList>

QVector<Product> CsvParser::parseProducts(const QString &filePath) {
    QVector<Product> products;
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть:" << filePath;
        return products;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    
    if (in.atEnd()) return products;
    in.readLine(); 
    
    int lineNum = 1;
    while (!in.atEnd()) {
        lineNum++;
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        
        QStringList f = line.split(';');
        if (f.size() < 6) {
            qWarning() << "Строка" << lineNum << ": мало полей";
            continue;
        }
        
        Product p;
        p.name = f[0].trimmed();
        p.category = f[1].trimmed();
        p.quantity = f[2].toInt();
        p.purchasePrice = f[3].toDouble();
        p.salePrice = f[4].toDouble();
        p.deliveryDate = QDate::fromString(f[5].trimmed(), "yyyy-MM-dd");
        
        if (!p.deliveryDate.isValid()) {
            qWarning() << "Строка" << lineNum << ": неверная дата";
            continue;
        }
        
        products.append(p);
    }
    
    file.close();
    qDebug() << "Загружено товаров:" << products.size();
    return products;
}

QMap<QString, QVector<int>> CsvParser::parseSales(const QString &filePath) {
    QMap<QString, QVector<int>> salesMap;
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть:" << filePath;
        return salesMap;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    
    if (in.atEnd()) return salesMap;
    in.readLine();
    
    int lineNum = 1;
    while (!in.atEnd()) {
        lineNum++;
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        
        QStringList f = line.split(';');
        if (f.size() < 7) {
            qWarning() << "Строка" << lineNum << ": мало полей";
            continue;
        }
        
        QString name = f[0].trimmed();
        QVector<int> sales;
        for (int i = 1; i <= 6; i++) {
            sales.append(f[i].toInt());
        }
        salesMap.insert(name, sales);
    }
    
    file.close();
    qDebug() << "Загружено продаж:" << salesMap.size();
    return salesMap;
}