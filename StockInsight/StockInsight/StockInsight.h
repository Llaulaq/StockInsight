#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QSplitter>
#include <QPushButton>
#include <QTabWidget>
#include <QLineEdit>
#include <QVector>
#include <QVBoxLayout>
#include <QComboBox>          // для фильтра по цвету

#include "models/Analytics.h"
#include "models/Product.h"

class StockInsight : public QMainWindow
{
    Q_OBJECT

public:
    StockInsight(QWidget* parent = nullptr);
    ~StockInsight() {}

    void setUserRole(const QString& role);  // Устанавливает роль и блокирует кнопки
    void setAnalytics(const Analytics& data, const QVector<Product>& products);  // Принимает данные от бэкенда

private slots:
    void loadCSV();                         // Загрузка CSV (будет удалена позже)
    void clearTable();                      // Очистка таблицы
    void onSearchTextChanged(const QString& text); // Поиск по таблице
    void logout();                          // Выход из учётной записи
    void showCharts();                      // Кнопка "Показать графики" — запускает Python-скрипт (будет заменена)
    void saveJSON();                        // Сохранение данных в JSON
    void exportJPEG();                      // Экспорт текущего графика в JPEG
    void sortByDays();                      // Сортировка таблицы по дням
    void filterByColor(int index);          // Фильтр таблицы по цвету строк
    void refreshData();                     // Обновить данные из CSV-файлов
    void toggleTheme();

private:
    QTableWidget* table;
    QPushButton* loadBtn;
    QPushButton* saveBtn;
    QPushButton* exportBtn;
    QPushButton* clearBtn;
    QPushButton* logoutBtn;
    QLineEdit* searchEdit;
    QComboBox* colorFilter;                 // Выпадающий список для фильтра по цвету
    QString currentRole;                    // Текущая роль пользователя
    QString currentCsvPath;                 // Путь к последнему загруженному CSV (будет удалён)
    QVector<QString> rowColors;             // Сохраняет цвета строк для фильтрации

    QTabWidget* tabs;                       // Вкладки для графиков
    QVector<QVBoxLayout*> chartLayouts;     // Layout'ы для вкладок, чтобы добавлять картинки

    bool isDarkTheme = true;

    Analytics currentAnalytics;             // Хранит аналитику от Димы
    QVector<Product> currentProducts;       // Хранит список товаров от Димы

    void runPythonScript(const QString& csvPath);   // Запускает Python-скрипт для генерации графиков (будет заменён)
    void loadChartsToTabs();                // Загружает готовые картинки графиков во вкладки (будет заменён)
    void loadChartsFromAnalytics();         // Строит графики из данных Analytics
};