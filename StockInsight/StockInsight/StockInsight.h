#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>

class StockInsight : public QMainWindow
{
    Q_OBJECT

public:
    StockInsight(QWidget* parent = nullptr);
    ~StockInsight() {}

    void setUserRole(const QString& role);  // Устанавливает роль и блокирует кнопки

private slots:
    void loadCSV();  // Загрузка CSV
    void clearTable();  // Очистка таблицы
    void onSearchTextChanged(const QString& text);  // Поиск по таблице
    void logout();  // Выход

private:
    QTableWidget* table;
    QPushButton* loadBtn;
    QPushButton* saveBtn;
    QPushButton* exportBtn;
    QPushButton* clearBtn;
    QPushButton* logoutBtn;
    QLineEdit* searchEdit;
    QString currentRole;
};