#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QSplitter>
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
    void loadCSV();                         // Загрузка CSV
    void clearTable();                      // Очистка таблицы
    void onSearchTextChanged(const QString& text); // Поиск по таблице
    void logout();                          // Выход из учётной записи
    void showCharts();                      // Кнопка "Показать графики" — запускает Python-скрипт
    void saveJSON();                        // Сохранение данных в JSON
    void exportJPEG();                      // Экспорт текущего графика в JPEG

private:
    QTableWidget* table;
    QPushButton* loadBtn;
    QPushButton* saveBtn;
    QPushButton* exportBtn;
    QPushButton* clearBtn;
    QPushButton* logoutBtn;
    QLineEdit* searchEdit;
    QString currentRole;                    // Текущая роль пользователя
    QString currentCsvPath;                 // Путь к последнему загруженному CSV

    QTabWidget* tabs;                       // Вкладки для графиков
    QVector<QVBoxLayout*> chartLayouts;     // Layout'ы для вкладок, чтобы добавлять картинки

    void runPythonScript(const QString& csvPath);   // Запускает Python-скрипт для генерации графиков
    void loadChartsToTabs();                // Загружает готовые картинки графиков во вкладки
};