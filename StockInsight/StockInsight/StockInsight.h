#pragma once

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QSplitter>
#include <QTableWidget>
#include <QTabWidget>
#include <QVector>
#include <QVBoxLayout>

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
    void setTheme(const QString& theme);                // Устанавливает тему (dark/light) и обновляет интерфейс
    void setUsername(const QString& username);          // Устанавливает имя пользователя для сохранения темы

    // Геттеры для main.cpp
    QString getCurrentRole() const { return currentRole; }
    int getProductCount() const { return currentProducts.size(); }

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;  // Обработчик событий для клика по графику

private slots:
    void loadCSV();                         // Загрузка CSV (оставлена для совместимости, не используется)
    void clearTable();                      // Очистка таблицы (только для администратора)
    void onSearchTextChanged(const QString& text);     // Поиск по таблице
    void logout();                          // Выход из учётной записи
    void showCharts();                      // Показать/удалить графики
    void saveJSON();                        // Сохранение данных в JSON (только видимые строки)
    void exportAllCharts();                 // Экспорт всех графиков в папку
    void refreshData();                     // Обновить данные из CSV-файлов (только для администратора)
    void toggleTheme();                     // Переключение тёмной/светлой темы
    void onChartClicked();                  // Обработчик клика по графику (открытие в отдельном окне)
    void onTabChanged(int index);           // Установка курсора при смене вкладки
    void onHeaderClicked(int column);       // Сортировка по клику на заголовок
    void filterByCategory(int index);       // Фильтр таблицы по категории
    void filterByColor(int index);          // Фильтр таблицы по цвету строк
    void exportCurrentChart(int index);     // Экспорт текущего графика из вкладки
    void resetFilters();                    // Сброс всех фильтров

private:
    // --- Виджеты интерфейса ---
    QTableWidget* table;                    // Таблица для отображения товаров
    QPushButton* loadBtn;                   // Кнопка загрузки CSV (не используется)
    QPushButton* saveBtn;                   // Сохранение JSON (над таблицей)
    QPushButton* exportAllBtn;              // Экспорт всех графиков
    QPushButton* clearBtn;                  // Очистка таблицы
    QPushButton* logoutBtn;                 // Выход из учётной записи
    QPushButton* refreshBtn;                // Обновление данных
    QPushButton* themeBtn;                  // Кнопка переключения темы
    QPushButton* showChartsBtn;             // Кнопка "Показать графики" / "Удалить графики"
    QPushButton* resetFiltersBtn;           // Кнопка сброса фильтров
    QLineEdit* searchEdit;                  // Поле поиска по таблице
    QComboBox* colorFilter;                 // Выпадающий список для фильтра по цвету
    QComboBox* categoryFilter;              // Выпадающий список для фильтра по категории

    // --- Виджеты статистики ---
    QLabel* totalProductsLabel;             // Всего товаров
    QLabel* totalProfitLabel;               // Суммарная прибыль
    QLabel* frozenMoneyLabel;               // Заморожено денег
    QLabel* deficitCountLabel;              // Дефицит
    QLabel* staleCountLabel;                // Залежалые

    // --- Данные ---
    QString currentRole;                    // Текущая роль пользователя
    QString currentUsername;                // Имя текущего пользователя для сохранения темы
    QString currentCsvPath;                 // Путь к последнему загруженному CSV (не используется)
    QVector<QString> rowColors;             // Сохраняет цвета строк для фильтрации
    bool isDarkTheme = true;                // Текущая тема: true — тёмная, false — светлая
    bool isTableCleared = false;            // Флаг: была ли таблица очищена
    bool chartsVisible = false;             // Флаг: показаны ли графики

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
    void loadSelectedCharts();              // Строит график по видимым товарам
    void saveThemeToFile(const QString& theme);     // Сохраняет тему пользователя в users.json
    void updateStatistics();                // Обновляет панель статистики
    void updateButtonsState();              // Обновляет состояние кнопок (активна/неактивна)
    void applyFilters();                    // Применение всех фильтров
    QVector<Product> getVisibleProducts() const;  // Получить только видимые товары (с учётом фильтров)
};