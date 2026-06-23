#include "StockInsight.h"
#include <QProcess>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QTabWidget>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QPixmap>
#include <QCoreApplication>


// КОНСТРУКТОР
StockInsight::StockInsight(QWidget* parent)
    : QMainWindow(parent), currentCsvPath("")   // ← инициализируем
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    // --- Верхняя панель ---
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    loadBtn = new QPushButton("📂 Загрузить CSV");
    saveBtn = new QPushButton("💾 Сохранить JSON");
    exportBtn = new QPushButton("🖼️ Экспорт JPEG");
    clearBtn = new QPushButton("🗑️ Очистить");
    logoutBtn = new QPushButton("🚪 Выйти");

    QPushButton* testBtn = new QPushButton("📈 Показать графики");

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("🔍 Поиск по товарам...");

    buttonLayout->addWidget(loadBtn);
    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(exportBtn);
    buttonLayout->addWidget(clearBtn);
    buttonLayout->addWidget(testBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(searchEdit);
    buttonLayout->addWidget(logoutBtn);

    mainLayout->addLayout(buttonLayout);

    // --- Таблица ---
    QLabel* tableLabel = new QLabel("📋 Список товаров");
    mainLayout->addWidget(tableLabel);

    table = new QTableWidget(0, 7);
    QStringList headers = { "Товар", "Категория", "Кол-во", "Цена зак.", "Цена прод.", "Дней", "Прибыль" };
    table->setHorizontalHeaderLabels(headers);
    mainLayout->addWidget(table);

    // --- Вкладки для графиков (с layout'ами для картинок) ---
    tabs = new QTabWidget();
    chartLayouts.clear();

    QStringList tabNames = { "📊 Остатки", "💰 Прибыль", "📈 Продажи", "⚠️ Залежалые" };
    for (int i = 0; i < tabNames.size(); ++i) {
        QWidget* page = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(page);
        page->setLayout(layout);
        tabs->addTab(page, tabNames[i]);
        chartLayouts.append(layout);
    }
    mainLayout->addWidget(tabs);

    setWindowTitle("📊 StockInsight — Анализ склада");
    resize(1000, 700);

    connect(loadBtn, &QPushButton::clicked, this, &StockInsight::loadCSV);
    connect(clearBtn, &QPushButton::clicked, this, &StockInsight::clearTable);
    connect(searchEdit, &QLineEdit::textChanged, this, &StockInsight::onSearchTextChanged);
    connect(logoutBtn, &QPushButton::clicked, this, &StockInsight::logout);
    connect(testBtn, &QPushButton::clicked, this, &StockInsight::showCharts);
}

// ЗАГРУЗКА CSV
void StockInsight::loadCSV()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Выберите CSV файл", "", "CSV файлы (*.csv);;Все файлы (*)");

    if (filePath.isEmpty()) {
        return;
    }

    currentCsvPath = filePath;   // ← сохраняем путь

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл!");
        return;
    }

    table->setRowCount(0);
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    QString line = stream.readLine();

    int row = 0;
    while (!stream.atEnd()) {
        line = stream.readLine();

        if (line.trimmed().isEmpty()) continue;
        line.remove('"');

        QStringList fields = line.split(';');

        if (fields.size() < 6) {
            continue;
        }

        table->insertRow(row);
        for (int col = 0; col < 6; ++col) {
            QTableWidgetItem* item = new QTableWidgetItem(fields[col].trimmed());
            table->setItem(row, col, item);
        }
        row++;
    }

    // --- Чередование строк и подсветка текста ---
    for (int r = 0; r < table->rowCount(); ++r) {
        int quantity = table->item(r, 2)->text().toInt();
        int days = table->item(r, 5)->text().toInt();

        // Определяем цвет текста
        QColor textColor;
        if (quantity < 5) {
            textColor = Qt::red;              // Дефицит
        }
        else if (days > 90) {
            textColor = QColor(255, 165, 0);  // Залежалый (оранжевый)
        }
        else if (quantity > 20) {
            textColor = Qt::darkGreen;        // Много товара (тёмно-зелёный)
        }
        else {
            textColor = Qt::white;            // Обычный текст (белый)
        }

        // Чередование фона
        QColor bgColor = (r % 2 == 0) ? QColor(50, 50, 50) : QColor(40, 40, 40);

        // Применяем ко всей строке
        for (int col = 0; col < table->columnCount(); ++col) {
            QTableWidgetItem* item = table->item(r, col);
            if (item) {
                item->setForeground(textColor);   // Цвет текста
                item->setBackground(bgColor);     // Чередование фона
            }
        }
    }

    file.close();
    QMessageBox::information(this, "Готово", "Загружено " + QString::number(row) + " товаров!");
}

// УСТАНОВКА РОЛИ (блокировка кнопок)
void StockInsight::setUserRole(const QString& role)
{
    currentRole = role;

    if (role == "guest") {
        loadBtn->setEnabled(false);
        saveBtn->setEnabled(false);
        exportBtn->setEnabled(false);
        clearBtn->setEnabled(false);
    }
    else if (role == "analyst") {
        loadBtn->setEnabled(true);
        saveBtn->setEnabled(true);
        exportBtn->setEnabled(true);
        clearBtn->setEnabled(false);
    }
    else { // admin
        loadBtn->setEnabled(true);
        saveBtn->setEnabled(true);
        exportBtn->setEnabled(true);
        clearBtn->setEnabled(true);
    }
}

// ОЧИСТКА ТАБЛИЦЫ (только для админа)
void StockInsight::clearTable()
{
    table->setRowCount(0);
    QMessageBox::information(this, "Готово", "Таблица очищена!");
}

// ПОИСК ПО ТАБЛИЦЕ 
void StockInsight::onSearchTextChanged(const QString& text)
{
    // Если поле пустое — показываем все строки
    if (text.isEmpty()) {
        for (int row = 0; row < table->rowCount(); ++row) {
            table->setRowHidden(row, false);
        }
        return;
    }

    // Ищем по всем строкам (по колонке "Товар" — индекс 0)
    for (int row = 0; row < table->rowCount(); ++row) {
        QTableWidgetItem* item = table->item(row, 0);
        bool isMatch = false;

        if (item) {
            QString cellText = item->text();
            if (cellText.contains(text, Qt::CaseInsensitive)) {
                isMatch = true;
            }
        }

        // Скрываем или показываем строку
        table->setRowHidden(row, !isMatch);
    }
}

// ЗАПУСК PYTHON-СКРИПТА ДЛЯ ГЕНЕРАЦИИ ГРАФИКОВ
void StockInsight::runPythonScript(const QString& csvPath)
{
    QString pythonExe = "python";
    QString scriptPath = QCoreApplication::applicationDirPath() + "/generate_charts.py";

    QProcess process;
    process.start(pythonExe, QStringList() << scriptPath << csvPath);
    process.waitForFinished();

    if (process.exitCode() == 0) {
        QMessageBox::information(this, "Готово", "Графики созданы!");
        // Загружаем полученные картинки во вкладки
        loadChartsToTabs();
    }
    else {
        QString error = process.readAllStandardError();
        QMessageBox::warning(this, "Ошибка", "Не удалось создать графики:\n" + error);
    }
}

// ЗАГРУЗКА КАРТИНОК ВО ВКЛАДКИ (после генерации Python-скриптом)
void StockInsight::loadChartsToTabs()
{
    // Очищаем старые виджеты во вкладках
    for (int i = 0; i < chartLayouts.size(); ++i) {
        QLayout* layout = chartLayouts[i];
        QLayoutItem* item;
        while ((item = layout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
    }

    QStringList imageFiles = {
        "chart_stock_by_category.jpeg",
        "chart_profit_by_category.jpeg",
        "chart_monthly_sales.jpeg",
        "chart_stale_products.jpeg"
    };

    for (int i = 0; i < imageFiles.size() && i < chartLayouts.size(); ++i) {
        // Путь к файлу в папке charts/
        QString imagePath = "charts/" + imageFiles[i];
        QPixmap pixmap(imagePath);
        QLabel* label = new QLabel();
        if (!pixmap.isNull()) {
            QWidget* parentWidget = chartLayouts[i]->parentWidget();
            int w = parentWidget->width() - 20;
            int h = parentWidget->height() - 20;
            if (w <= 0) w = 600;
            if (h <= 0) h = 400;
            label->setPixmap(pixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            label->setAlignment(Qt::AlignCenter);
        }
        else {
            label->setText("График не найден: " + imagePath);
        }
        chartLayouts[i]->addWidget(label);
    }
}

// ТЕСТОВАЯ ФУНКЦИЯ ДЛЯ ГРАФИКОВ (вызывается по кнопке)
void StockInsight::showCharts()
{
    if (currentCsvPath.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Сначала загрузите CSV-файл!");
        return;
    }
    runPythonScript(currentCsvPath);
}

// ВЫХОД ИЗ УЧЁТНОЙ ЗАПИСИ
void StockInsight::logout()
{
    this->close();  // Закрывает главное окно
}