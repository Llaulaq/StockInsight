#include "StockInsight.h"
#include "LoginDialog.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    while (true) {
        LoginDialog login;
        if (login.exec() == QDialog::Accepted) {
            QString role = login.getRole();

            StockInsight w;
            w.setUserRole(role);
            w.setWindowTitle("📊 StockInsight — Анализ склада [" + role + "]");
            w.show();

            // Показываем приветствие с ролью
            QMessageBox::information(nullptr, "Добро пожаловать",
                "Вы вошли как: " + role + "\nДоступные функции зависят от вашей роли.");

            // Ждём, пока главное окно закроется
            app.exec();

            // Если окно закрыто — цикл повторяется (снова показываем вход)
        }
        else {
            // Пользователь нажал "Отмена" — выходим из программы
            return 0;
        }
    }
}