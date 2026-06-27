#include "StockInsight.h"
#include "LoginDialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QDebug>          
#include <QFile>
#include <QDir>
#include <QCoreApplication>

#include "models/Product.h"
#include "models/Analytics.h"
#include "services/CsvParser.h"
#include "services/DataMerger.h"
#include "services/AnalyticsEngine.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    while (true) {
        LoginDialog login;
        if (login.exec() == QDialog::Accepted) {
            QString role = login.getRole();

            // Получаем путь к папке с .exe
            QString exePath = QCoreApplication::applicationDirPath();
            QString productsPath = exePath + "/products.csv";
            QString salesPath = exePath + "/sales.csv";

            qDebug() << "Путь к .exe:" << exePath;
            qDebug() << "products.csv exists:" << QFile::exists(productsPath);
            qDebug() << "sales.csv exists:" << QFile::exists(salesPath);

            // ---- ЗАГРУЖАЕМ ДАННЫЕ ЧЕРЕЗ БЭКЕНД ДИМЫ ----
            QVector<Product> products = CsvParser::parseProducts(productsPath);
            auto salesMap = CsvParser::parseSales(salesPath);
            DataMerger::merge(products, salesMap);
            Analytics analytics = AnalyticsEngine::calculate(products);

            // ---- ПОКАЗЫВАЕМ ГЛАВНОЕ ОКНО С ДАННЫМИ ----
            StockInsight w;
            w.setUserRole(role);
            w.setAnalytics(analytics, products);

            QMessageBox::information(nullptr, "Добро пожаловать",
                "Вы вошли как: " + role + "\nДанные загружены: " + QString::number(products.size()) + " товаров.");

            w.show();

            app.exec();

        }
        else {
            return 0;
        }
    }
}