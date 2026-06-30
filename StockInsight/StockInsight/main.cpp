#include "StockInsight.h"
#include "LoginDialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QScreen>

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

            QString exePath = QCoreApplication::applicationDirPath();
            QString productsPath = exePath + "/products.csv";
            QString salesPath = exePath + "/sales.csv";

            qDebug() << "Путь к .exe:" << exePath;
            qDebug() << "products.csv exists:" << QFile::exists(productsPath);
            qDebug() << "sales.csv exists:" << QFile::exists(salesPath);

            QVector<Product> products = CsvParser::parseProducts(productsPath);
            auto salesMap = CsvParser::parseSales(salesPath);
            DataMerger::merge(products, salesMap);
            Analytics analytics = AnalyticsEngine::calculate(products);

            StockInsight w;
            w.setUserRole(role);
            w.setAnalytics(analytics, products);

            // --- ПОКАЗЫВАЕМ ОКНО ---
            w.show();

            // --- ЦЕНТРИРУЕМ ОКНО ПОСЛЕ ОТОБРАЖЕНИЯ ---
            QScreen* screen = QGuiApplication::primaryScreen();
            if (screen) {
                QRect screenGeometry = screen->availableGeometry();
                int x = (screenGeometry.width() - w.width()) / 2;
                int y = (screenGeometry.height() - w.height()) / 2;
                w.move(x, y);
            }

            QMessageBox::information(nullptr, "Добро пожаловать",
                "Вы вошли как: " + role + "\nДанные загружены: " + QString::number(products.size()) + " товаров.");

            app.exec();

        }
        else {
            return 0;
        }
    }
}