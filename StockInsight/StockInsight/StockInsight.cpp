#include "StockInsight.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPixmap>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QSet>
#include <QSplitter>
#include <QTableWidget>
#include <QTabWidget>
#include <QTextStream>
#include <QVBoxLayout>

#include "models/Analytics.h"
#include "models/Product.h"
#include "services/AnalyticsEngine.h"
#include "services/CsvParser.h"
#include "services/DataMerger.h"

// КОНСТРУКТОР
StockInsight::StockInsight(QWidget* parent)
    : QMainWindow(parent), currentCsvPath(""), isTableCleared(false), chartsVisible(false)
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    // --- Панель статистики ---
    QHBoxLayout* statsLayout = new QHBoxLayout();

    totalProductsLabel = new QLabel("📦 Всего: 0");
    totalProductsLabel->setToolTip("Общее количество товаров на складе");

    totalProfitLabel = new QLabel("💰 Прибыль: 0 ₽");
    totalProfitLabel->setToolTip("Суммарная потенциальная прибыль от продажи всех товаров");

    frozenMoneyLabel = new QLabel("❄️ Заморожено: 0 ₽");
    frozenMoneyLabel->setToolTip("Средства, замороженные в залежалых товарах");

    deficitCountLabel = new QLabel("🔴 Дефицит: 0");
    deficitCountLabel->setToolTip("Количество товаров с риском дефицита");

    staleCountLabel = new QLabel("🟠 Залежалые: 0");
    staleCountLabel->setToolTip("Количество залежалых товаров (более 90 дней на складе)");

    // Стили для статистики
    QString statsStyle = "font-size: 13px; font-weight: bold; padding: 4px 10px;";
    totalProductsLabel->setStyleSheet(statsStyle);
    totalProfitLabel->setStyleSheet(statsStyle);
    frozenMoneyLabel->setStyleSheet(statsStyle);
    deficitCountLabel->setStyleSheet(statsStyle);
    staleCountLabel->setStyleSheet(statsStyle);

    statsLayout->addWidget(totalProductsLabel);
    statsLayout->addWidget(totalProfitLabel);
    statsLayout->addWidget(frozenMoneyLabel);
    statsLayout->addWidget(deficitCountLabel);
    statsLayout->addWidget(staleCountLabel);
    statsLayout->addStretch();

    mainLayout->addLayout(statsLayout);

    // --- Верхняя панель ---
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    resetFiltersBtn = new QPushButton("🔄 Сбросить фильтры");
    resetFiltersBtn->setToolTip("Сбросить все фильтры и поиск");
    resetFiltersBtn->setMaximumWidth(150);

    exportAllBtn = new QPushButton("📦 Экспорт всех графиков");
    exportAllBtn->setToolTip("Сохранить все графики в выбранную папку");

    clearBtn = new QPushButton("🗑️ Очистить");
    clearBtn->setToolTip("Очистить таблицу (только для администратора)");

    logoutBtn = new QPushButton("🚪 Выйти");
    logoutBtn->setToolTip("Выйти из учётной записи");

    showChartsBtn = new QPushButton("📈 Показать графики");
    showChartsBtn->setToolTip("Сгенерировать и показать графики");

    refreshBtn = new QPushButton("🔄 Обновить данные");
    refreshBtn->setToolTip("Обновить данные из CSV-файлов (только для администратора)");

    themeBtn = new QPushButton("🌙 Тёмная");
    themeBtn->setToolTip("Переключить тему (светлая/тёмная)");

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("🔍 Поиск по товарам...");
    searchEdit->setToolTip("Введите текст для поиска по названию товара");

    // --- Фильтр по категории ---
    categoryFilter = new QComboBox();
    categoryFilter->addItem("Все категории");
    categoryFilter->setMaximumWidth(150);
    categoryFilter->setToolTip("Фильтр по категории");

    // --- Фильтр по цвету ---
    colorFilter = new QComboBox();
    colorFilter->addItems({ "Все товары", "⚠️ Дефицит", "⏳ Залежалые", "📦 Избыток", "✅ Норма" });
    colorFilter->setMaximumWidth(150);
    colorFilter->setToolTip("Фильтр по состоянию товара");

    buttonLayout->addWidget(resetFiltersBtn);
    buttonLayout->addWidget(exportAllBtn);
    buttonLayout->addWidget(clearBtn);
    buttonLayout->addWidget(showChartsBtn);
    buttonLayout->addWidget(categoryFilter);
    buttonLayout->addWidget(colorFilter);
    buttonLayout->addWidget(refreshBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(searchEdit);
    buttonLayout->addWidget(logoutBtn);
    buttonLayout->addWidget(themeBtn);

    mainLayout->addLayout(buttonLayout);

    // --- Заголовок таблицы с кнопкой сохранения ---
    QHBoxLayout* tableHeaderLayout = new QHBoxLayout();

    QLabel* tableLabel = new QLabel("📋 Список товаров");
    saveBtn = new QPushButton("💾 Сохранить JSON");
    saveBtn->setToolTip("Сохранить данные из текущей таблицы в JSON-файл (с учётом фильтров)");
    saveBtn->setEnabled(false);

    tableHeaderLayout->addWidget(tableLabel);
    tableHeaderLayout->addStretch();
    tableHeaderLayout->addWidget(saveBtn);

    mainLayout->addLayout(tableHeaderLayout);

    // --- Таблица ---
    table = new QTableWidget(0, 7);
    QStringList headers = { "Товар", "Категория", "Кол-во", "Цена зак.", "Цена прод.", "Дней", "Прибыль" };
    table->setHorizontalHeaderLabels(headers);
    table->setAlternatingRowColors(true);
    table->setMouseTracking(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Подключаем сортировку по клику на заголовок
    connect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &StockInsight::onHeaderClicked);

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

    // Подключаем сигнал смены вкладки для установки курсора
    connect(tabs, &QTabWidget::currentChanged, this, &StockInsight::onTabChanged);

    // Устанавливаем курсор-руку для всех вкладок
    for (int i = 0; i < tabs->count(); ++i) {
        QWidget* page = tabs->widget(i);
        page->setCursor(Qt::PointingHandCursor);
    }

    // Добавляем оба виджета в разделитель
    splitter->addWidget(tableWrapper);
    splitter->addWidget(chartsWrapper);

    // Начальные размеры
    splitter->setSizes({ 600, 400 });

    mainLayout->addWidget(splitter);

    setWindowTitle("📊 StockInsight — Анализ склада");
    resize(1000, 700);

    // --- Загрузка стилей (тёмная тема по умолчанию) ---
    QFile styleFile("styles/dark.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = styleFile.readAll();
        this->setStyleSheet(style);
        styleFile.close();
    }

    // --- Подключение сигналов к слотам ---
    connect(clearBtn, &QPushButton::clicked, this, &StockInsight::clearTable);
    connect(searchEdit, &QLineEdit::textChanged, this, &StockInsight::onSearchTextChanged);
    connect(logoutBtn, &QPushButton::clicked, this, &StockInsight::logout);
    connect(showChartsBtn, &QPushButton::clicked, this, &StockInsight::showCharts);
    connect(saveBtn, &QPushButton::clicked, this, &StockInsight::saveJSON);
    connect(exportAllBtn, &QPushButton::clicked, this, &StockInsight::exportAllCharts);
    connect(resetFiltersBtn, &QPushButton::clicked, this, &StockInsight::resetFilters);
    connect(categoryFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StockInsight::filterByCategory);
    connect(colorFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StockInsight::filterByColor);
    connect(refreshBtn, &QPushButton::clicked, this, &StockInsight::refreshData);
    connect(themeBtn, &QPushButton::clicked, this, &StockInsight::toggleTheme);

    // Обновляем статистику при запуске
    updateStatistics();
    updateButtonsState();
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
    table->resizeColumnsToContents();
    QMessageBox::information(this, "Готово", "Загружено " + QString::number(row) + " товаров!");
}

// УСТАНОВКА РОЛИ (блокировка кнопок)
void StockInsight::setUserRole(const QString& role)
{
    currentRole = role;
    updateButtonsState();
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

    // Удаляем графики, если они показаны
    if (chartsVisible) {
        for (int i = 0; i < chartLayouts.size(); ++i) {
            QLayout* layout = chartLayouts[i];
            QLayoutItem* item;
            while ((item = layout->takeAt(0)) != nullptr) {
                delete item->widget();
                delete item;
            }
        }
        chartsVisible = false;
        showChartsBtn->setText("📈 Показать графики");
        showChartsBtn->setToolTip("Сгенерировать и показать графики");
    }

    table->setRowCount(0);
    rowColors.clear();
    isTableCleared = true;   // ← запоминаем, что таблица очищена
    updateStatistics();      // Обновляем статистику
    updateButtonsState();    // Блокируем кнопки
    QMessageBox::information(this, "Готово", "Таблица и графики очищены!");
}

// ПОИСК ПО ТАБЛИЦЕ
void StockInsight::onSearchTextChanged(const QString& text)
{
    if (text.isEmpty()) {
        for (int row = 0; row < table->rowCount(); ++row) {
            table->setRowHidden(row, false);
        }
        updateButtonsState();
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
    updateButtonsState();
}

// ЗАПУСК PYTHON-СКРИПТА
void StockInsight::runPythonScript(const QString& csvPath)
{
    // Пытаемся найти python.exe рядом с программой
    QString pythonExe = QCoreApplication::applicationDirPath() + "/python.exe";

    // Если рядом нет — используем системный python
    if (!QFile::exists(pythonExe)) {
        pythonExe = "python";
    }

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

// ЗАГРУЗКА КАРТИНОК ВО ВКЛАДКИ С КНОПКОЙ СОХРАНЕНИЯ
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

        // --- КНОПКА СОХРАНЕНИЯ (ПЕРЕД ГРАФИКОМ) ---
        QPushButton* saveChartBtn = new QPushButton("💾 Сохранить JPEG");
        saveChartBtn->setToolTip("Сохранить этот график в JPEG-файл");

        // Добавляем кнопку в layout
        chartLayouts[i]->addWidget(saveChartBtn, 0, Qt::AlignCenter);

        // Подключаем сигнал
        connect(saveChartBtn, &QPushButton::clicked, this, [this, i]() {
            exportCurrentChart(i);
            });

        // --- ГРАФИК ---
        QLabel* label = new QLabel();
        if (!pixmap.isNull()) {
            QWidget* parentWidget = chartLayouts[i]->parentWidget();
            int w = parentWidget->width() - 20;
            int h = parentWidget->height() - 20;

            if (w < 700) w = 700;
            if (h < 500) h = 500;

            label->setPixmap(pixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            label->setAlignment(Qt::AlignCenter);
            label->installEventFilter(this);
        }
        else {
            label->setText("График не найден: " + imagePath);
        }
        chartLayouts[i]->addWidget(label);
    }

    chartsVisible = true;
    showChartsBtn->setText("🗑️ Удалить графики");
    showChartsBtn->setToolTip("Удалить графики из вкладок");
}

// КНОПКА "ПОКАЗАТЬ ГРАФИКИ" / "УДАЛИТЬ ГРАФИКИ"
void StockInsight::showCharts()
{
    // Если графики уже показаны — удаляем их
    if (chartsVisible) {
        // Очищаем все вкладки
        for (int i = 0; i < chartLayouts.size(); ++i) {
            QLayout* layout = chartLayouts[i];
            QLayoutItem* item;
            while ((item = layout->takeAt(0)) != nullptr) {
                delete item->widget();
                delete item;
            }
        }

        chartsVisible = false;
        showChartsBtn->setText("📈 Показать графики");
        showChartsBtn->setToolTip("Сгенерировать и показать графики");
        return;
    }

    // Проверяем, есть ли данные и не очищена ли таблица
    if (currentProducts.isEmpty() || isTableCleared) {
        QMessageBox::warning(this, "Ошибка", "Нет данных для построения графиков! Сначала загрузите данные.");
        return;
    }

    // Если данные есть — строим графики
    loadChartsFromAnalytics();
}

// ФИЛЬТР ПО КАТЕГОРИИ
void StockInsight::filterByCategory(int index)
{
    applyFilters();
    updateButtonsState();
}

// ФИЛЬТР ПО ЦВЕТУ
void StockInsight::filterByColor(int index)
{
    applyFilters();
    updateButtonsState();
}

// СБРОС ВСЕХ ФИЛЬТРОВ
void StockInsight::resetFilters()
{
    categoryFilter->setCurrentIndex(0);
    colorFilter->setCurrentIndex(0);
    searchEdit->clear();

    applyFilters();
    updateButtonsState();
}

// ПРИМЕНЕНИЕ ВСЕХ ФИЛЬТРОВ
void StockInsight::applyFilters()
{
    if (table->rowCount() == 0) {
        return;
    }

    QString category = categoryFilter->currentText();
    QString color = colorFilter->currentText();

    for (int row = 0; row < table->rowCount(); ++row) {
        bool show = true;

        // Фильтр по категории
        if (category != "Все категории") {
            QTableWidgetItem* item = table->item(row, 1);
            if (item) {
                show = (item->text() == category);
            }
        }

        // Фильтр по цвету
        if (show && color != "Все статусы" && color != "Показать все" && color != "Все товары") {
            if (color == "⚠️ Дефицит") {
                show = (rowColors[row] == "deficit");
            }
            else if (color == "⏳ Залежалые") {
                show = (rowColors[row] == "stale");
            }
            else if (color == "📦 Избыток") {
                show = (rowColors[row] == "green");
            }
            else if (color == "✅ Норма") {
                show = (rowColors[row] == "normal");
            }
        }

        table->setRowHidden(row, !show);
    }
}

// СОХРАНЕНИЕ ДАННЫХ В JSON
void StockInsight::saveJSON()
{
    // Получаем только видимые товары
    QVector<Product> visibleProducts = getVisibleProducts();

    if (visibleProducts.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Нет данных для сохранения! Таблица пуста или все строки скрыты фильтром.");
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(this, "Сохранить JSON", "analytics.json", "JSON файлы (*.json)");
    if (filePath.isEmpty()) return;

    // Заново рассчитываем аналитику только для видимых товаров
    Analytics visibleAnalytics = AnalyticsEngine::calculate(visibleProducts);

    QJsonArray productsArray;

    for (const Product& p : visibleProducts) {
        QJsonObject product;
        product["name"] = p.name;
        product["category"] = p.category;
        product["quantity"] = p.quantity;
        product["purchase_price"] = p.purchasePrice;
        product["sale_price"] = p.salePrice;
        product["days_in_stock"] = p.daysInStock;
        product["total_profit"] = p.totalProfit;
        product["is_deficit"] = p.isDeficit;
        product["is_stale"] = p.isStale;

        QJsonArray salesArr;
        for (int s : p.monthlySales) {
            salesArr.append(s);
        }
        product["monthly_sales"] = salesArr;

        productsArray.append(product);
    }

    // --- Сводка из visibleAnalytics ---
    QJsonObject summary;
    summary["total_potential_profit"] = visibleAnalytics.totalPotentialProfit;
    summary["frozen_money"] = visibleAnalytics.frozenMoney;
    summary["deficit_risk_count"] = visibleAnalytics.deficitRiskCount;
    summary["stale_count"] = visibleAnalytics.staleCount;

    // --- Данные для графиков ---
    QJsonObject chartsData;

    QJsonObject stockCat;
    for (auto it = visibleAnalytics.stockByCategory.begin(); it != visibleAnalytics.stockByCategory.end(); ++it) {
        stockCat[it.key()] = it.value();
    }
    chartsData["stock_by_category"] = stockCat;

    QJsonObject profitCat;
    for (auto it = visibleAnalytics.profitByCategory.begin(); it != visibleAnalytics.profitByCategory.end(); ++it) {
        profitCat[it.key()] = it.value();
    }
    chartsData["profit_by_category"] = profitCat;

    QJsonObject salesMonth;
    for (auto it = visibleAnalytics.salesByMonth.begin(); it != visibleAnalytics.salesByMonth.end(); ++it) {
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

// ЭКСПОРТ ТЕКУЩЕГО ГРАФИКА
void StockInsight::exportCurrentChart(int index)
{
    if (currentProducts.isEmpty() || isTableCleared) {
        QMessageBox::warning(this, "Ошибка", "Нет данных для экспорта!");
        return;
    }

    if (index < 0 || index >= chartLayouts.size()) {
        return;
    }

    QLayout* layout = chartLayouts[index];
    if (!layout || layout->count() < 2) {  // 0 - кнопка, 1 - график
        QMessageBox::warning(this, "Ошибка", "В этой вкладке нет графика.");
        return;
    }

    // Ищем QLabel с графиком (второй виджет в layout)
    QWidget* widget = layout->itemAt(1)->widget();
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

    QString tabName = tabs->tabText(index);
    QString defaultName = QString("chart_%1_%2.jpeg")
        .arg(index + 1)
        .arg(tabName.remove(QRegularExpression("[📊💰📈⚠️]")).trimmed());

    QString filePath = QFileDialog::getSaveFileName(this, "Сохранить JPEG", defaultName, "JPEG файлы (*.jpeg)");
    if (filePath.isEmpty()) return;

    if (pixmap.save(filePath, "JPEG", 95)) {
        QMessageBox::information(this, "Готово", "JPEG сохранён!\n" + filePath);
    }
    else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить JPEG!");
    }
}

// ЭКСПОРТ ВСЕХ ГРАФИКОВ В ПАПКУ
void StockInsight::exportAllCharts()
{
    // Проверяем, есть ли данные
    if (currentProducts.isEmpty() || isTableCleared) {
        QMessageBox::warning(this, "Ошибка", "Нет данных для экспорта!");
        return;
    }

    // Проверяем, существуют ли графики
    QStringList imageFiles = {
        "chart_stock_by_category.jpeg",
        "chart_profit_by_category.jpeg",
        "chart_monthly_sales.jpeg",
        "chart_stale_products.jpeg"
    };

    // Проверяем, есть ли хотя бы один график
    bool hasCharts = false;
    for (const QString& file : imageFiles) {
        QString fullPath = "charts/" + file;
        if (QFile::exists(fullPath)) {
            hasCharts = true;
            break;
        }
    }

    if (!hasCharts) {
        QMessageBox::warning(this, "Ошибка", "Графики не найдены! Сначала создайте графики.");
        return;
    }

    // Только после проверки предлагаем выбрать папку
    QString dirPath = QFileDialog::getExistingDirectory(this, "Выберите папку для сохранения графиков");
    if (dirPath.isEmpty()) return;

    QStringList tabNames = { "Остатки", "Прибыль", "Продажи", "Залежалые" };
    int saved = 0;

    for (int i = 0; i < imageFiles.size(); ++i) {
        QString sourcePath = "charts/" + imageFiles[i];

        // Проверяем, существует ли файл
        if (!QFile::exists(sourcePath)) {
            continue;
        }

        // Формируем имя файла с понятным названием
        QString destPath = dirPath + "/" + tabNames[i] + "_" + imageFiles[i];

        if (QFile::copy(sourcePath, destPath)) {
            saved++;
        }
    }

    if (saved > 0) {
        QMessageBox::information(this, "Готово",
            "Экспортировано " + QString::number(saved) + " графиков в папку:\n" + dirPath);
    }
    else {
        QMessageBox::warning(this, "Ошибка", "Не удалось экспортировать графики!");
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
        QTableWidgetItem* nameItem = new QTableWidgetItem(p.name);
        nameItem->setData(Qt::DisplayRole, p.name);
        table->setItem(row, 0, nameItem);

        QTableWidgetItem* categoryItem = new QTableWidgetItem(p.category);
        categoryItem->setData(Qt::DisplayRole, p.category);
        table->setItem(row, 1, categoryItem);

        // -- Числовые поля --
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

    // Обновляем список категорий для фильтра
    categoryFilter->blockSignals(true);
    categoryFilter->clear();
    categoryFilter->addItem("Все категории");

    QSet<QString> categories;
    for (const Product& p : products) {
        categories.insert(p.category);
    }

    for (const QString& cat : categories) {
        categoryFilter->addItem(cat);
    }
    categoryFilter->blockSignals(false);

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
    table->resizeColumnsToContents();

    // Обновляем статистику и состояние кнопок
    updateStatistics();
    updateButtonsState();
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
    table->resizeColumnsToContents();

    // Если графики были показаны — пересоздаём их
    if (chartsVisible) {
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

// УСТАНАВЛИВАЕТ ТЕМУ (dark/light)
void StockInsight::setTheme(const QString& theme)
{
    if (theme == "light") {
        isDarkTheme = false;
        QFile styleFile("styles/light.qss");
        if (styleFile.open(QFile::ReadOnly)) {
            QString style = styleFile.readAll();
            this->setStyleSheet(style);
            styleFile.close();
        }
        if (themeBtn) {
            themeBtn->setText("☀️ Светлая");
        }
    }
    else {
        isDarkTheme = true;
        QFile styleFile("styles/dark.qss");
        if (styleFile.open(QFile::ReadOnly)) {
            QString style = styleFile.readAll();
            this->setStyleSheet(style);
            styleFile.close();
        }
        if (themeBtn) {
            themeBtn->setText("🌙 Тёмная");
        }
    }
}

// УСТАНАВЛИВАЕТ ИМЯ ПОЛЬЗОВАТЕЛЯ
void StockInsight::setUsername(const QString& username)
{
    currentUsername = username;
}

// СОХРАНЯЕТ ТЕМУ В users.json
void StockInsight::saveThemeToFile(const QString& theme)
{
    if (currentUsername.isEmpty()) {
        return;
    }

    QFile file("users.json");
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    QJsonArray usersArray = root["users"].toArray();

    for (int i = 0; i < usersArray.size(); ++i) {
        QJsonObject obj = usersArray[i].toObject();
        if (obj["username"].toString() == currentUsername) {
            obj["theme"] = theme;
            usersArray[i] = obj;
            break;
        }
    }

    root["users"] = usersArray;

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        file.close();
    }
}

// ПЕРЕКЛЮЧЕНИЕ ТЕМ (тёмная/светлая)
void StockInsight::toggleTheme()
{
    isDarkTheme = !isDarkTheme;

    QString themeFile = isDarkTheme ? "styles/dark.qss" : "styles/light.qss";
    QFile styleFile(themeFile);
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = styleFile.readAll();
        this->setStyleSheet(style);
        styleFile.close();
    }

    // Меняем текст кнопки
    if (themeBtn) {
        themeBtn->setText(isDarkTheme ? "🌙 Тёмная" : "☀️ Светлая");
    }

    // Сохраняем тему в users.json
    saveThemeToFile(isDarkTheme ? "dark" : "light");

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

// ОБРАБОТЧИК СОБЫТИЙ ДЛЯ КЛИКА ПО ГРАФИКУ
bool StockInsight::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            QLabel* label = qobject_cast<QLabel*>(obj);
            if (label) {
                QPixmap pixmap = label->pixmap();
                if (!pixmap.isNull()) {
                    onChartClicked();
                    return true;
                }
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

// ОТКРЫТИЕ ГРАФИКА В ОТДЕЛЬНОМ ОКНЕ
void StockInsight::onChartClicked()
{
    int currentTab = tabs->currentIndex();
    if (currentTab < 0 || currentTab >= chartLayouts.size()) {
        return;
    }

    QLayout* layout = chartLayouts[currentTab];
    if (!layout || layout->count() < 2) {  // 0 - кнопка, 1 - график
        return;
    }

    // Ищем QLabel с графиком (второй виджет в layout)
    QWidget* widget = layout->itemAt(1)->widget();
    QLabel* label = qobject_cast<QLabel*>(widget);
    if (!label) {
        return;
    }

    QPixmap pixmap = label->pixmap();
    if (pixmap.isNull()) {
        return;
    }

    // Создаём отдельное окно для увеличенного графика
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("📊 " + tabs->tabText(currentTab));
    dialog->setModal(false);
    dialog->resize(900, 700);

    QVBoxLayout* dialogLayout = new QVBoxLayout(dialog);

    // --- УВЕЛИЧЕННЫЙ ГРАФИК ---
    QLabel* bigLabel = new QLabel();
    QPixmap scaled = pixmap.scaled(880, 680, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    bigLabel->setPixmap(scaled);
    bigLabel->setAlignment(Qt::AlignCenter);
    bigLabel->setStyleSheet("background-color: #1e1e2f;");

    dialogLayout->addWidget(bigLabel);

    // --- КНОПКА СОХРАНЕНИЯ (под графиком) ---
    QPushButton* saveBigChartBtn = new QPushButton("💾 Сохранить JPEG");
    saveBigChartBtn->setToolTip("Сохранить этот график в JPEG-файл");

    // Подключаем сигнал (сохраняем текущий график)
    connect(saveBigChartBtn, &QPushButton::clicked, this, [this, currentTab]() {
        exportCurrentChart(currentTab);
        });

    // --- КНОПКА ЗАКРЫТИЯ ---
    QPushButton* closeBtn = new QPushButton("✖ Закрыть");
    closeBtn->setFixedWidth(120);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(saveBigChartBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);

    dialogLayout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    connect(dialog, &QDialog::rejected, dialog, &QDialog::accept);

    dialog->exec();
    delete dialog;
}

// УСТАНОВКА КУРСОРА ПРИ СМЕНЕ ВКЛАДКИ
void StockInsight::onTabChanged(int index)
{
    QWidget* page = tabs->widget(index);
    if (page) {
        page->setCursor(Qt::PointingHandCursor);
    }
}

// ОБНОВЛЕНИЕ ПАНЕЛИ СТАТИСТИКИ
void StockInsight::updateStatistics()
{
    if (currentProducts.isEmpty()) {
        totalProductsLabel->setText("📦 Всего: 0");
        totalProfitLabel->setText("💰 Прибыль: 0 ₽");
        frozenMoneyLabel->setText("❄️ Заморожено: 0 ₽");
        deficitCountLabel->setText("🔴 Дефицит: 0");
        staleCountLabel->setText("🟠 Залежалые: 0");
        return;
    }

    int total = currentProducts.size();
    double totalProfit = 0;
    double frozenMoney = 0;
    int deficit = 0;
    int stale = 0;

    for (const Product& p : currentProducts) {
        totalProfit += p.totalProfit;
        if (p.isDeficit) deficit++;
        if (p.isStale) {
            stale++;
            frozenMoney += p.quantity * p.purchasePrice;
        }
    }

    totalProductsLabel->setText("📦 Всего: " + QString::number(total));
    totalProfitLabel->setText("💰 Прибыль: " + QString::number(totalProfit, 'f', 0) + " ₽");
    frozenMoneyLabel->setText("❄️ Заморожено: " + QString::number(frozenMoney, 'f', 0) + " ₽");
    deficitCountLabel->setText("🔴 Дефицит: " + QString::number(deficit));
    staleCountLabel->setText("🟠 Залежалые: " + QString::number(stale));
}

// ОБНОВЛЕНИЕ СОСТОЯНИЯ КНОПОК
void StockInsight::updateButtonsState()
{
    bool hasData = !currentProducts.isEmpty() && !isTableCleared;
    bool hasVisibleData = !getVisibleProducts().isEmpty();

    // Кнопка сохранения JSON (над таблицей)
    saveBtn->setEnabled(hasVisibleData);

    // Кнопки экспорта графиков
    bool canExport = hasData && (currentRole == "admin" || currentRole == "analyst");
    exportAllBtn->setEnabled(canExport);

    // Кнопка показа/удаления графиков — активна только если есть данные
    showChartsBtn->setEnabled(hasData);

    // Кнопка сброса фильтров — активна всегда
    resetFiltersBtn->setEnabled(true);

    // Кнопки, зависящие только от роли
    clearBtn->setEnabled(currentRole == "admin");
    refreshBtn->setEnabled(currentRole == "admin");
}

// ПОЛУЧЕНИЕ ТОЛЬКО ВИДИМЫХ ТОВАРОВ
QVector<Product> StockInsight::getVisibleProducts() const
{
    QVector<Product> visibleProducts;

    if (currentProducts.isEmpty() || isTableCleared) {
        return visibleProducts;
    }

    for (int row = 0; row < table->rowCount(); ++row) {
        if (!table->isRowHidden(row) && row < currentProducts.size()) {
            visibleProducts.append(currentProducts[row]);
        }
    }

    return visibleProducts;
}

// СОРТИРОВКА ПО КЛИКУ НА ЗАГОЛОВОК
void StockInsight::onHeaderClicked(int column)
{
    static int lastColumn = -1;
    static Qt::SortOrder order = Qt::AscendingOrder;

    if (lastColumn == column) {
        order = (order == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    }
    else {
        lastColumn = column;
        order = Qt::AscendingOrder;
    }

    table->sortItems(column, order);
}