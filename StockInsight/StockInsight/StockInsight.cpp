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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSplitter>          
#include <QComboBox>          


// КОНСТРУКТОР
StockInsight::StockInsight(QWidget* parent)
    : QMainWindow(parent), currentCsvPath("")
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
    QPushButton* sortBtn = new QPushButton("📅 Сортировать по дням");       

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("🔍 Поиск по товарам...");

    // --- Фильтр по цвету (выпадающий список) ---
    colorFilter = new QComboBox();
    colorFilter->addItems({ "Все", "🔴 Дефицит", "🟠 Залежалые", "🟢 Много товара" });
    colorFilter->setMaximumWidth(150);

    buttonLayout->addWidget(loadBtn);
    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(exportBtn);
    buttonLayout->addWidget(clearBtn);
    buttonLayout->addWidget(testBtn);
    buttonLayout->addWidget(sortBtn);              
    buttonLayout->addWidget(colorFilter);         
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

    // --- Разделитель между таблицей и графиками ---
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);

    // Обёртка для таблицы
    QWidget* tableWrapper = new QWidget();
    QVBoxLayout* tableWrapperLayout = new QVBoxLayout(tableWrapper);
    tableWrapperLayout->setContentsMargins(0, 0, 0, 0);
    tableWrapperLayout->addWidget(table);
    tableWrapper->setMinimumHeight(150);       // чтобы таблица не схлопывалась

    // Обёртка для вкладок с графиками
    QWidget* chartsWrapper = new QWidget();
    QVBoxLayout* chartsWrapperLayout = new QVBoxLayout(chartsWrapper);
    chartsWrapperLayout->setContentsMargins(0, 0, 0, 0);
    chartsWrapper->setMinimumHeight(150);      // чтобы графики не схлопывались

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
    chartsWrapperLayout->addWidget(tabs);

    // Добавляем оба виджета в разделитель
    splitter->addWidget(tableWrapper);
    splitter->addWidget(chartsWrapper);

    // Начальные размеры (60% таблица, 40% графики)
    splitter->setSizes({ 600, 400 });

    // Добавляем разделитель в основной layout
    mainLayout->addWidget(splitter);

    setWindowTitle("📊 StockInsight — Анализ склада");
    resize(1000, 700);

    // Подключение сигналов к слотам
    connect(loadBtn, &QPushButton::clicked, this, &StockInsight::loadCSV);
    connect(clearBtn, &QPushButton::clicked, this, &StockInsight::clearTable);
    connect(searchEdit, &QLineEdit::textChanged, this, &StockInsight::onSearchTextChanged);
    connect(logoutBtn, &QPushButton::clicked, this, &StockInsight::logout);
    connect(testBtn, &QPushButton::clicked, this, &StockInsight::showCharts);
    connect(saveBtn, &QPushButton::clicked, this, &StockInsight::saveJSON);
    connect(exportBtn, &QPushButton::clicked, this, &StockInsight::exportJPEG);
    connect(sortBtn, &QPushButton::clicked, this, &StockInsight::sortByDays);           // Сортировка
    connect(colorFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StockInsight::filterByColor); // Фильтр по цвету
}

// ЗАГРУЗКА CSV
void StockInsight::loadCSV()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Выберите CSV файл", "", "CSV файлы (*.csv);;Все файлы (*)");

    if (filePath.isEmpty()) {
        return;
    }

    currentCsvPath = filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл!");
        return;
    }

    table->setRowCount(0);
    // Очищаем сохранённые цвета строк
    rowColors.clear();

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
        QString colorKey = "normal";
        if (quantity < 5) {
            textColor = Qt::red;              // Дефицит
            colorKey = "deficit";
        }
        else if (days > 90) {
            textColor = QColor(255, 165, 0);  // Залежалый (оранжевый)
            colorKey = "stale";
        }
        else if (quantity > 20) {
            textColor = Qt::darkGreen;        // Много товара (тёмно-зелёный)
            colorKey = "many";
        }
        else {
            textColor = Qt::white;            // Обычный текст (белый)
        }

        // Сохраняем цвет строки
        rowColors.append(colorKey);

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

    // --- АДМИН ---
    if (role == "admin") {
        loadBtn->setEnabled(true);
        saveBtn->setEnabled(true);
        exportBtn->setEnabled(true);
        clearBtn->setEnabled(true);
    }
    // --- АНАЛИТИК ---
    else if (role == "analyst") {
        loadBtn->setEnabled(true);
        saveBtn->setEnabled(true);
        exportBtn->setEnabled(true);
        clearBtn->setEnabled(false);   // Аналитик НЕ может очищать таблицу
    }
    // --- ГОСТЬ ---
    else if (role == "guest") {
        loadBtn->setEnabled(false);    // Гость НЕ может загружать
        saveBtn->setEnabled(false);    // Гость НЕ может сохранять JSON
        exportBtn->setEnabled(false);  // Гость НЕ может экспортировать JPEG
        clearBtn->setEnabled(false);   // Гость НЕ может очищать таблицу
    }
}

// ОЧИСТКА ТАБЛИЦЫ (только для админа)
void StockInsight::clearTable()
{
    table->setRowCount(0);
    rowColors.clear();
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
        loadChartsToTabs();
    }
    else {
        QString error = process.readAllStandardError();
        QMessageBox::warning(this, "Ошибка", "Не удалось создать графики:\n" + error);
    }
}

// ЗАГРУЗКА КАРТИНОК ВО ВКЛАДКИ
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

// КНОПКА "ПОКАЗАТЬ ГРАФИКИ"
void StockInsight::showCharts()
{
    if (currentCsvPath.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Сначала загрузите CSV-файл!");
        return;
    }
    runPythonScript(currentCsvPath);
}

// СОРТИРОВКА ПО ДНЯМ
void StockInsight::sortByDays()
{
    static bool ascending = true;
    table->sortItems(5, ascending ? Qt::AscendingOrder : Qt::DescendingOrder);
    ascending = !ascending;
}

// ФИЛЬТР ПО ЦВЕТУ
void StockInsight::filterByColor(int index)
{
    QString filterText = colorFilter->currentText();

    for (int row = 0; row < table->rowCount(); ++row) {
        // Определяем, соответствует ли строка фильтру
        bool show = true;

        if (filterText == "🔴 Дефицит") {
            show = (rowColors[row] == "deficit");
        }
        else if (filterText == "🟠 Залежалые") {
            show = (rowColors[row] == "stale");
        }
        else if (filterText == "🟢 Много товара") {
            show = (rowColors[row] == "many");
        }
        else { // "Все"
            show = true;
        }

        table->setRowHidden(row, !show);
    }
}

// СОХРАНЕНИЕ ДАННЫХ В JSON
void StockInsight::saveJSON()
{
    if (table->rowCount() == 0) {
        QMessageBox::warning(this, "Ошибка", "Таблица пуста! Сначала загрузите CSV.");
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(this, "Сохранить JSON", "analytics.json", "JSON файлы (*.json)");
    if (filePath.isEmpty()) return;

    QJsonArray productsArray;
    QJsonObject summary;
    QJsonObject chartsData;
    QJsonObject stockByCategory;
    QJsonObject profitByCategory;

    double totalProfit = 0;
    int deficitCount = 0;
    int staleCount = 0;

    for (int row = 0; row < table->rowCount(); ++row) {
        QJsonObject product;

        // Проверяем наличие ячеек
        QTableWidgetItem* nameItem = table->item(row, 0);
        QTableWidgetItem* categoryItem = table->item(row, 1);
        QTableWidgetItem* quantityItem = table->item(row, 2);
        QTableWidgetItem* purchaseItem = table->item(row, 3);
        QTableWidgetItem* saleItem = table->item(row, 4);
        QTableWidgetItem* daysItem = table->item(row, 5);

        // Пропускаем строку, если какая-то ячейка пуста
        if (!nameItem || !categoryItem || !quantityItem || !purchaseItem || !saleItem || !daysItem) {
            continue;
        }

        // Безопасное преобразование с проверкой на пустоту
        QString nameText = nameItem->text().trimmed();
        QString categoryText = categoryItem->text().trimmed();
        QString quantityText = quantityItem->text().trimmed();
        QString purchaseText = purchaseItem->text().trimmed();
        QString saleText = saleItem->text().trimmed();
        QString daysText = daysItem->text().trimmed();

        if (nameText.isEmpty() || categoryText.isEmpty() || quantityText.isEmpty() ||
            purchaseText.isEmpty() || saleText.isEmpty() || daysText.isEmpty()) {
            continue;
        }

        bool okQty, okPur, okSale, okDays;
        int quantity = quantityText.toInt(&okQty);
        double purchasePrice = purchaseText.toDouble(&okPur);
        double salePrice = saleText.toDouble(&okSale);
        int daysInStock = daysText.toInt(&okDays);

        // Если преобразование не удалось — пропускаем строку
        if (!okQty || !okPur || !okSale || !okDays) {
            continue;
        }

        product["name"] = nameText;
        product["category"] = categoryText;
        product["quantity"] = quantity;
        product["purchase_price"] = purchasePrice;
        product["sale_price"] = salePrice;
        product["days_in_stock"] = daysInStock;

        double profit = (salePrice - purchasePrice) * quantity;
        product["profit_per_unit"] = profit / quantity;
        product["total_profit"] = profit;
        totalProfit += profit;

        product["is_stale"] = daysInStock > 90;
        product["is_deficit"] = quantity < 5;

        if (product["is_deficit"].toBool()) deficitCount++;
        if (product["is_stale"].toBool()) staleCount++;

        QString category = categoryText;
        stockByCategory[category] = stockByCategory[category].toInt() + quantity;
        profitByCategory[category] = profitByCategory[category].toDouble() + profit;

        productsArray.append(product);
    }

    summary["total_potential_profit"] = totalProfit;
    summary["deficit_risk_count"] = deficitCount;
    summary["stale_count"] = staleCount;

    chartsData["stock_by_category"] = stockByCategory;
    chartsData["profit_by_category"] = profitByCategory;

    QJsonObject root;
    root["summary"] = summary;
    root["products"] = productsArray;
    root["charts_data"] = chartsData;

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        file.close();
        QMessageBox::information(this, "Готово", "JSON сохранён!\n" + filePath);
    }
    else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить JSON!");
    }
}

// ЭКСПОРТ ТЕКУЩЕГО ГРАФИКА В JPEG
void StockInsight::exportJPEG()
{
    int currentTab = tabs->currentIndex();
    if (currentTab < 0 || currentTab >= chartLayouts.size()) {
        QMessageBox::warning(this, "Ошибка", "Нет активной вкладки с графиком.");
        return;
    }

    QLayout* layout = chartLayouts[currentTab];
    if (!layout || layout->count() == 0) {
        QMessageBox::warning(this, "Ошибка", "В этой вкладке нет графика.");
        return;
    }

    QWidget* widget = layout->itemAt(0)->widget();
    QLabel* label = qobject_cast<QLabel*>(widget);
    if (!label) {
        QMessageBox::warning(this, "Ошибка", "Не удалось получить график.");
        return;
    }

    // Получаем QPixmap по значению
    QPixmap pixmap = label->pixmap();
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "Ошибка", "В этой вкладке нет графика для экспорта.");
        return;
    }

    QString defaultName = QString("chart_%1.jpeg").arg(currentTab + 1);
    QString filePath = QFileDialog::getSaveFileName(this, "Сохранить JPEG", defaultName, "JPEG файлы (*.jpeg)");
    if (filePath.isEmpty()) return;

    if (pixmap.save(filePath, "JPEG", 95)) {
        QMessageBox::information(this, "Готово", "JPEG сохранён!\n" + filePath);
    }
    else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить JPEG!");
    }
}

// ВЫХОД ИЗ УЧЁТНОЙ ЗАПИСИ
void StockInsight::logout()
{
    this->close();
}