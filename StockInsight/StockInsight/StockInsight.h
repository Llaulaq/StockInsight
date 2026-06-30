#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QSplitter>
#include <QPushButton>
#include <QTabWidget>
#include <QLineEdit>
#include <QVector>
#include <QVBoxLayout>
#include <QComboBox>          

#include "models/Analytics.h"
#include "models/Product.h"

class StockInsight : public QMainWindow
{
    Q_OBJECT

public:
    StockInsight(QWidget* parent = nullptr);
    ~StockInsight() {}

    void setUserRole(const QString& role);              // Устанавливает роль и блокирует кнопки
    void setAnalytics(const Analytics& data, const QVector<Product>& products);  // Принимает данные от бэкенда

private slots:
    void loadCSV();                         // Загрузка CSV (оставлена для совместимости, не используется)
    void clearTable();                      // Очистка таблицы (только для администратора)
    void onSearchTextChanged(const QString& text);     // Поиск по таблице
    void logout();                          // Выход из учётной записи
    void showCharts();                      // Показать графики
    void saveJSON();                        // Сохранение данных в JSON
    void exportJPEG();                      // Экспорт текущего графика в JPEG
    void sortByDays();                      // Сортировка таблицы по дням
    void filterByColor(int index);          // Фильтр таблицы по цвету строк
    void refreshData();                     // Обновить данные из CSV-файлов (только для администратора)
    void toggleTheme();                     // Переключение тёмной/светлой темы

private:
    // --- Виджеты интерфейса ---
    QTableWidget* table;                    // Таблица для отображения товаров
    QPushButton* loadBtn;                   // Кнопка загрузки CSV (не используется)
    QPushButton* saveBtn;                   // Сохранение JSON
    QPushButton* exportBtn;                 // Экспорт JPEG
    QPushButton* clearBtn;                  // Очистка таблицы
    QPushButton* logoutBtn;                 // Выход из учётной записи
    QPushButton* refreshBtn;                // Обновление данных
    QLineEdit* searchEdit;                  // Поле поиска по таблице
    QComboBox* colorFilter;                 // Выпадающий список для фильтра по цвету

    // --- Данные ---
    QString currentRole;                    // Текущая роль пользователя
    QString currentCsvPath;                 // Путь к последнему загруженному CSV (не используется)
    QVector<QString> rowColors;             // Сохраняет цвета строк для фильтрации
    bool isDarkTheme = true;                // Текущая тема: true — тёмная, false — светлая
    bool isTableCleared = false;            // Флаг: была ли таблица очищена

    // --- Вкладки и графики ---
    QTabWidget* tabs;                       // Вкладки для графиков
    QVector<QVBoxLayout*> chartLayouts;     // Layout'ы для вкладок, чтобы добавлять картинки

    // --- Данные от бэкенда ---
    Analytics currentAnalytics;             // Хранит аналитику от Димы
    QVector<Product> currentProducts;       // Хранит список товаров от Димы

    // --- Вспомогательные методы ---
    void runPythonScript(const QString& csvPath);   // Запускает Python-скрипт для генерации графиков
    void loadChartsToTabs();                // Загружает готовые картинки графиков во вкладки
    void loadChartsFromAnalytics();         // Строит графики из данных Analytics
};