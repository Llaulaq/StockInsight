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

#include "models/Product.h"
#include "models/Analytics.h"
#include "services/CsvParser.h"
#include "services/DataMerger.h"
#include "services/AnalyticsEngine.h"

// КОНСТРУКТОР
StockInsight::StockInsight(QWidget* parent)
    : QMainWindow(parent), currentCsvPath(""), isTableCleared(false)
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    // --- Верхняя панель ---
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    // loadBtn = new QPushButton("📂 Загрузить CSV");  // ← УДАЛЕНО (больше не нужно)
    saveBtn = new QPushButton("💾 Сохранить JSON");
    exportBtn = new QPushButton("🖼️ Экспорт JPEG");
    clearBtn = new QPushButton("🗑️ Очистить");
    logoutBtn = new QPushButton("🚪 Выйти");

    QPushButton* testBtn = new QPushButton("📈 Показать графики");
    QPushButton* sortBtn = new QPushButton("📅 Сортировать по дням");
    refreshBtn = new QPushButton("🔄 Обновить данные");

    QPushButton* themeBtn = new QPushButton("🌙 Тёмная");
    themeBtn->setToolTip("Переключить тему (светлая/тёмная)");

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("🔍 Поиск по товарам...");

    // --- Фильтр по цвету (выпадающий список) ---
    colorFilter = new QComboBox();
    colorFilter->addItems({ "Все", "🔴 Дефицит", "🟠 Залежалые", "🟢 Много товара", "⚪ Обычные" });
    colorFilter->setMaximumWidth(150);

    // --- ПОДСКАЗКИ ДЛЯ КНОПОК ---
    saveBtn->setToolTip("Сохранить данные в JSON-файл");
    exportBtn->setToolTip("Экспортировать текущий график в JPEG");
    clearBtn->setToolTip("Очистить таблицу (только для администратора)");
    logoutBtn->setToolTip("Выйти из учётной записи");
    testBtn->setToolTip("Сгенерировать и показать графики");
    sortBtn->setToolTip("Сортировать таблицу по количеству дней на складе");
    refreshBtn->setToolTip("Обновить данные из CSV-файлов");
    searchEdit->setToolTip("Введите текст для поиска по товарам");
    colorFilter->setToolTip("Фильтровать строки по цвету");

    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(exportBtn);
    buttonLayout->addWidget(clearBtn);
    buttonLayout->addWidget(testBtn);
    buttonLayout->addWidget(sortBtn);
    buttonLayout->addWidget(colorFilter);
    buttonLayout->addWidget(refreshBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(searchEdit);
    buttonLayout->addWidget(logoutBtn);
    buttonLayout->addWidget(themeBtn);

    mainLayout->addLayout(buttonLayout);

    // --- Таблица ---
    QLabel* tableLabel = new QLabel("📋 Список товаров");
    mainLayout->addWidget(tableLabel);

    table = new QTableWidget(0, 7);
    QStringList headers = { "Товар", "Категория", "Кол-во", "Цена зак.", "Цена прод.", "Дней", "Прибыль" };
    table->setHorizontalHeaderLabels(headers);
    table->setAlternatingRowColors(true);

    // --- Разделитель между таблицей и графиками ---
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);

    // Обёртка для таблицы
    QWidget* tableWrapper = new QWidget();
    QVBoxLayout* tableWrapperLayout = new QVBoxLayout(tableWrapper);
    tableWrapperLayout->setContentsMargins(0, 0, 0, 0);
    tableWrapperLayout->addWidget(table);
    tableWrapper->setMinimumHeight(150);

    // Обёртка для вкладок с графиками
    QWidget* chartsWrapper = new QWidget();
    QVBoxLayout* chartsWrapperLayout = new QVBoxLayout(chartsWrapper);
    chartsWrapperLayout->setContentsMargins(0, 0, 0, 0);
    chartsWrapper->setMinimumHeight(150);

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

    // Начальные размеры
    splitter->setSizes({ 600, 400 });

    mainLayout->addWidget(splitter);

    setWindowTitle("📊 StockInsight — Анализ склада");
    resize(1000, 700);

    // --- Загрузка стилей (тёмная тема по умолчанию) ---
    QFile styleFile("style_dark.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = styleFile.readAll();
        this->setStyleSheet(style);
        styleFile.close();
    }

    // --- Подключение сигналов к слотам ---
    // connect(loadBtn, &QPushButton::clicked, this, &StockInsight::loadCSV);  // ← УДАЛЕНО
    connect(clearBtn, &QPushButton::clicked, this, &StockInsight::clearTable);
    connect(searchEdit, &QLineEdit::textChanged, this, &StockInsight::onSearchTextChanged);
    connect(logoutBtn, &QPushButton::clicked, this, &StockInsight::logout);
    connect(testBtn, &QPushButton::clicked, this, &StockInsight::showCharts);
    connect(saveBtn, &QPushButton::clicked, this, &StockInsight::saveJSON);
    connect(exportBtn, &QPushButton::clicked, this, &StockInsight::exportJPEG);
    connect(sortBtn, &QPushButton::clicked, this, &StockInsight::sortByDays);
    connect(colorFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StockInsight::filterByColor);
    connect(refreshBtn, &QPushButton::clicked, this, &StockInsight::refreshData);
    connect(themeBtn, &QPushButton::clicked, this, &StockInsight::toggleTheme);
}

// ЗАГРУЗКА CSV (оставлена для совместимости, но не используется)
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

        QColor textColor;
        QString colorKey = "normal";
        if (quantity < 5) {
            textColor = Qt::red;
            colorKey = "deficit";
        }
        else if (days > 90) {
            textColor = QColor(255, 165, 0);
            colorKey = "stale";
        }
        else if (quantity > 20) {
            textColor = Qt::darkGreen;
            colorKey = "many";
        }
        else {
            textColor = Qt::white;
        }

        rowColors.append(colorKey);

        QColor bgColor = (r % 2 == 0) ? QColor(50, 50, 50) : QColor(40, 40, 40);

        for (int col = 0; col < table->columnCount(); ++col) {
            QTableWidgetItem* item = table->item(r, col);
            if (item) {
                item->setForeground(textColor);
                item->setBackground(bgColor);
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

    if (role == "admin") {
        // loadBtn->setEnabled(true);
        saveBtn->setEnabled(true);
        exportBtn->setEnabled(true);
        clearBtn->setEnabled(true);
        refreshBtn->setEnabled(true);
    }
    else if (role == "analyst") {
        // loadBtn->setEnabled(true);
        saveBtn->setEnabled(true);
        exportBtn->setEnabled(true);
        clearBtn->setEnabled(false);
        refreshBtn->setEnabled(false);   // ← аналитик НЕ может обновлять данные
    }
    else if (role == "guest") {
        // loadBtn->setEnabled(false);
        saveBtn->setEnabled(false);
        exportBtn->setEnabled(false);
        clearBtn->setEnabled(false);
        refreshBtn->setEnabled(false);
    }
}

// ОЧИСТКА ТАБЛИЦЫ (только для админа)
void StockInsight::clearTable()
{
    // Проверяем, есть ли у пользователя права
    if (currentRole != "admin") {
        QMessageBox::warning(this, "Доступ запрещён",
            "Только администратор может очищать таблицу!");
        return;
    }

    table->setRowCount(0);
    rowColors.clear();
    isTableCleared = true;   // ← запоминаем, что таблица очищена
    QMessageBox::information(this, "Готово", "Таблица очищена!");
}

// ПОИСК ПО ТАБЛИЦЕ
void StockInsight::onSearchTextChanged(const QString& text)
{
    if (text.isEmpty()) {
        for (int row = 0; row < table->rowCount(); ++row) {
            table->setRowHidden(row, false);
        }
        return;
    }

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

// ЗАПУСК PYTHON-СКРИПТА
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

            // --- Минимальный размер, чтобы график не был слишком маленьким ---
            if (w < 700) w = 700;
            if (h < 500) h = 500;

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
    // Проверяем, есть ли данные
    if (currentProducts.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Нет данных для построения графиков! Сначала загрузите CSV.");
        return;
    }

    if (!currentProducts.isEmpty()) {
        loadChartsFromAnalytics();
        return;
    }

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
        bool show = true;

        if (filterText == "🔴 Дефицит") {
            show = (rowColors[row] == "deficit");
        }
        else if (filterText == "🟠 Залежалые") {
            show = (rowColors[row] == "stale");
        }
        else if (filterText == "🟢 Много товара") {
            show = (rowColors[row] == "green");
        }
        else if (filterText == "⚪ Обычные") {
            show = (rowColors[row] == "normal");
        }
        else {
            show = true;
        }

        table->setRowHidden(row, !show);
    }
}

// СОХРАНЕНИЕ ДАННЫХ В JSON
void StockInsight::saveJSON()
{
    if (currentProducts.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Нет данных для сохранения! Сначала загрузите CSV.");
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(this, "Сохранить JSON", "analytics.json", "JSON файлы (*.json)");
    if (filePath.isEmpty()) return;

    QJsonArray productsArray;

    // --- Товары из currentProducts ---
    for (const Product& p : currentProducts) {
        QJsonObject product;
        product["name"] = p.name;
        product["category"] = p.category;
        product["quantity"] = p.quantity;
        product["purchase_price"] = p.purchasePrice;
        product["sale_price"] = p.salePrice;
        product["days_in_stock"] = p.daysInStock;
        product["total_profit"] = p.totalProfit;
        product["is_defisit"] = p.isDeficit;
        product["is_stale"] = p.isStale;

        // monthlySales (если есть)
        QJsonArray salesArr;
        for (int s : p.monthlySales) {
            salesArr.append(s);
        }
        product["monthly_sales"] = salesArr;

        productsArray.append(product);
    }

    // --- Сводка из currentAnalytics ---
    QJsonObject summary;
    summary["total_potential_profit"] = currentAnalytics.totalPotentialProfit;
    summary["frozen_money"] = currentAnalytics.frozenMoney;
    summary["defisit_risk_count"] = currentAnalytics.deficitRiskCount;
    summary["stale_count"] = currentAnalytics.staleCount;

    // --- Данные для графиков ---
    QJsonObject chartsData;

    QJsonObject stockCat;
    for (auto it = currentAnalytics.stockByCategory.begin(); it != currentAnalytics.stockByCategory.end(); ++it) {
        stockCat[it.key()] = it.value();
    }
    chartsData["stock_by_category"] = stockCat;

    QJsonObject profitCat;
    for (auto it = currentAnalytics.profitByCategory.begin(); it != currentAnalytics.profitByCategory.end(); ++it) {
        profitCat[it.key()] = it.value();
    }
    chartsData["profit_by_category"] = profitCat;

    QJsonObject salesMonth;
    for (auto it = currentAnalytics.salesByMonth.begin(); it != currentAnalytics.salesByMonth.end(); ++it) {
        QJsonArray arr;
        for (int s : it.value()) {
            arr.append(s);
        }
        salesMonth[it.key()] = arr;
    }
    chartsData["sales_by_month"] = salesMonth;

    // --- Корень JSON ---
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
    // Проверяем, есть ли данные
    if (currentProducts.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Нет данных для экспорта! Сначала загрузите CSV.");
        return;
    }

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

// ПЕРЕДАЧА ДАННЫХ ОТ БЭКЕНДА
void StockInsight::setAnalytics(const Analytics& data, const QVector<Product>& products)
{
    currentAnalytics = data;
    currentProducts = products;

    // Если таблица была очищена — не заполняем её заново
    if (isTableCleared) {
        return;
    }

    table->setRowCount(0);
    rowColors.clear();

    for (const Product& p : products) {
        int row = table->rowCount();
        table->insertRow(row);

        // -- Текстовые поля --
        table->setItem(row, 0, new QTableWidgetItem(p.name));
        table->setItem(row, 1, new QTableWidgetItem(p.category));

        // -- Числовые поля (для корректной сортировки) --
        QTableWidgetItem* qtyItem = new QTableWidgetItem();
        qtyItem->setData(Qt::DisplayRole, p.quantity);
        table->setItem(row, 2, qtyItem);

        QTableWidgetItem* purchaseItem = new QTableWidgetItem();
        purchaseItem->setData(Qt::DisplayRole, p.purchasePrice);
        table->setItem(row, 3, purchaseItem);

        QTableWidgetItem* saleItem = new QTableWidgetItem();
        saleItem->setData(Qt::DisplayRole, p.salePrice);
        table->setItem(row, 4, saleItem);

        QTableWidgetItem* daysItem = new QTableWidgetItem();
        daysItem->setData(Qt::DisplayRole, p.daysInStock);
        table->setItem(row, 5, daysItem);

        QTableWidgetItem* profitItem = new QTableWidgetItem();
        profitItem->setData(Qt::DisplayRole, p.totalProfit);
        table->setItem(row, 6, profitItem);

        // -- Цветовая метка --
        if (p.isDeficit) {
            rowColors.append("deficit");
        }
        else if (p.isStale) {
            rowColors.append("stale");
        }
        else if (p.quantity > 20) {
            rowColors.append("green");
        }
        else {
            rowColors.append("normal");
        }
    }

    // -- Применяем цвета --
    // Определяем цвет для обычных товаров в зависимости от темы
    QColor normalColor = isDarkTheme ? Qt::white : Qt::black;

    for (int r = 0; r < table->rowCount(); ++r) {
        QColor textColor;
        if (rowColors[r] == "deficit") {
            textColor = Qt::red;
        }
        else if (rowColors[r] == "stale") {
            textColor = QColor(255, 165, 0);
        }
        else if (rowColors[r] == "green") {
            textColor = Qt::darkGreen;
        }
        else {
            textColor = normalColor;
        }

        for (int col = 0; col < table->columnCount(); ++col) {
            QTableWidgetItem* item = table->item(r, col);
            if (item) {
                item->setForeground(textColor);
            }
        }
    }

    // Принудительно обновляем таблицу, чтобы цвета точно применились
    table->viewport()->update();
}

// ОБНОВЛЕНИЕ ДАННЫХ 
void StockInsight::refreshData()
{
    // Проверяем роль
    if (currentRole != "admin") {
        QMessageBox::warning(this, "Доступ запрещён",
            "Только администратор может обновлять данные!");
        return;
    }

    QString exePath = QCoreApplication::applicationDirPath();
    QString productsPath = exePath + "/products.csv";
    QString salesPath = exePath + "/sales.csv";

    qDebug() << "Обновление данных...";
    qDebug() << "products.csv exists:" << QFile::exists(productsPath);
    qDebug() << "sales.csv exists:" << QFile::exists(salesPath);

    // Проверяем, есть ли файлы
    if (!QFile::exists(productsPath)) {
        QMessageBox::warning(this, "Ошибка", "Файл products.csv не найден!\n" + productsPath);
        return;
    }

    // Загружаем данные через бэкенд Димы
    QVector<Product> products = CsvParser::parseProducts(productsPath);
    auto salesMap = CsvParser::parseSales(salesPath);
    DataMerger::merge(products, salesMap);
    Analytics analytics = AnalyticsEngine::calculate(products);

    // При загрузке новых данных сбрасываем флаг очистки
    isTableCleared = false;

    // Обновляем таблицу и графики
    setAnalytics(analytics, products);

    // Если есть данные — обновляем графики
    if (!products.isEmpty()) {
        loadChartsFromAnalytics();
    }

    QMessageBox::information(this, "Готово",
        "Данные обновлены!\nЗагружено " + QString::number(products.size()) + " товаров.");
}

// СТРОИТ ГРАФИКИ ИЗ ДАННЫХ ANALYTICS (через Python)
void StockInsight::loadChartsFromAnalytics()
{
    for (int i = 0; i < chartLayouts.size(); ++i) {
        QLayout* layout = chartLayouts[i];
        QLayoutItem* item;
        while ((item = layout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
    }

    if (currentProducts.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Нет данных для построения графиков!");
        return;
    }

    QFile file("temp_data.csv");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "Товар;Категория;Количество;Цена_закупки;Цена_продажи;Дней\n";
        for (const Product& p : currentProducts) {
            out << p.name << ";" << p.category << ";" << p.quantity << ";"
                << p.purchasePrice << ";" << p.salePrice << ";" << p.daysInStock << "\n";
        }
        file.close();
    }

    runPythonScript("temp_data.csv");
}

// ПЕРЕКЛЮЧЕНИЕ ТЕМ (тёмная/светлая)
void StockInsight::toggleTheme()
{
    isDarkTheme = !isDarkTheme;

    QString themeFile = isDarkTheme ? "style_dark.qss" : "style_light.qss";
    QFile styleFile(themeFile);
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = styleFile.readAll();
        this->setStyleSheet(style);
        styleFile.close();
    }

    // Меняем текст кнопки
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (btn) {
        btn->setText(isDarkTheme ? "🌙 Тёмная" : "☀️ Светлая");
    }

    // Если таблица НЕ была очищена и есть данные — обновляем цвета
    if (!isTableCleared && !currentProducts.isEmpty()) {
        setAnalytics(currentAnalytics, currentProducts);
    }
}

// ВЫХОД ИЗ УЧЁТНОЙ ЗАПИСИ
void StockInsight::logout()
{
    this->close();  // Закрывает главное окно
}