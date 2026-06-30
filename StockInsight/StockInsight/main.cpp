#include "StockInsight.h"
#include "LoginDialog.h"
#include "RegisterDialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QScreen>
#include <QThread>

#include "models/Product.h"
#include "models/Analytics.h"
#include "services/CsvParser.h"
#include "services/DataMerger.h"
#include "services/AnalyticsEngine.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Создаём окна один раз для всего цикла
    LoginDialog login;
    RegisterDialog reg;
    StockInsight mainWindow;

    // Бесконечный цикл, пока пользователь не закроет программу
    while (true) {
        // Показываем окно входа
        int result = login.exec();

        // Если пользователь нажал "Отмена" или закрыл окно → выходим из программы
        if (result != QDialog::Accepted) {
            break;
        }

        // Если пользователь успешно вошёл
        QString role = login.getRole();
        QString theme = login.getTheme();   // Получаем тему пользователя
        QString username = login.getUsername(); // Получаем имя пользователя

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

        mainWindow.setUserRole(role);
        mainWindow.setUsername(username);        // Передаём имя пользователя для сохранения темы
        mainWindow.setAnalytics(analytics, products);
        mainWindow.setTheme(theme);              // Устанавливаем тему пользователя

        // --- ПОКАЗЫВАЕМ ОКНО ---
        mainWindow.show();

        // --- ЦЕНТРИРУЕМ ОКНО ПОСЛЕ ОТОБРАЖЕНИЯ ---
        QScreen* screen = QGuiApplication::primaryScreen();
        if (screen) {
            QRect screenGeometry = screen->availableGeometry();
            int x = (screenGeometry.width() - mainWindow.width()) / 2;
            int y = (screenGeometry.height() - mainWindow.height()) / 2;
            mainWindow.move(x, y);
        }

        QMessageBox::information(nullptr, "Добро пожаловать",
            "Вы вошли как: " + mainWindow.getCurrentRole() + "\nДанные загружены: " + QString::number(mainWindow.getProductCount()) + " товаров.");

        // Ждём, пока главное окно не закроется
        while (mainWindow.isVisible()) {
            app.processEvents();
            QThread::msleep(100);
        }

        // После закрытия главного окна скрываем его и продолжаем цикл
        mainWindow.hide();
    }

    return 0;
}