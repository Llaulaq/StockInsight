#include "StockInsight.h"
#include "LoginDialog.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    LoginDialog login;
    if (login.exec() == QDialog::Accepted) {
        QString role = login.getRole();

        StockInsight w;
        w.setWindowTitle("📊 StockInsight — Анализ склада [" + role + "]");
        w.show();

        // Показываем приветствие с ролью
        QMessageBox::information(nullptr, "Добро пожаловать",
            "Вы вошли как: " + role + "\nДоступные функции зависят от вашей роли.");

        return app.exec();
    }
    else {
        return 0; // Пользователь нажал "Отмена"
    }
}