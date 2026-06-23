#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QTabWidget>
#include <QLineEdit>
#include <QVector>
#include <QVBoxLayout>

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
    void showCharts();  // Кнопка "Показать графики" — запускает Python-скрипт

private:
    QTableWidget* table;
    QPushButton* loadBtn;
    QPushButton* saveBtn;
    QPushButton* exportBtn;
    QPushButton* clearBtn;
    QPushButton* logoutBtn;
    QLineEdit* searchEdit;
    QString currentRole;
    QString currentCsvPath;            // ← путь к последнему загруженному CSV

    QTabWidget* tabs;                 // Вкладки для графиков
    QVector<QVBoxLayout*> chartLayouts; // Layout'ы для вкладок

    void runPythonScript(const QString& csvPath);  // Запускает Python-скрипт
    void loadChartsToTabs();                       // Загружает картинки во вкладки
};