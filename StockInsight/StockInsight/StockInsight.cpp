#include "StockInsight.h"
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

StockInsight::StockInsight(QWidget* parent)
    : QMainWindow(parent)
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    // --- Верхняя панель ---
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    QPushButton* loadBtn = new QPushButton("📂 Загрузить CSV");
    QPushButton* saveBtn = new QPushButton("💾 Сохранить JSON");
    QPushButton* exportBtn = new QPushButton("🖼️ Экспорт JPEG");

    QLineEdit* searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("🔍 Поиск по товарам...");

    buttonLayout->addWidget(loadBtn);
    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(exportBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(searchEdit);

    mainLayout->addLayout(buttonLayout);

    // --- Таблица ---
    QLabel* tableLabel = new QLabel("📋 Список товаров");
    mainLayout->addWidget(tableLabel);

    table = new QTableWidget(0, 7);
    QStringList headers = { "Товар", "Категория", "Кол-во", "Цена зак.", "Цена прод.", "Дней", "Прибыль" };
    table->setHorizontalHeaderLabels(headers);
    table->setAlternatingRowColors(true);
    mainLayout->addWidget(table);

    // --- Вкладки ---
    QTabWidget* tabs = new QTabWidget();
    tabs->addTab(new QWidget(), "📊 Остатки");
    tabs->addTab(new QWidget(), "💰 Прибыль");
    tabs->addTab(new QWidget(), "📈 Продажи");
    tabs->addTab(new QWidget(), "⚠️ Залежалые");
    mainLayout->addWidget(tabs);

    setWindowTitle("📊 StockInsight — Анализ склада");
    resize(1000, 700);

    connect(loadBtn, &QPushButton::clicked, this, &StockInsight::loadCSV);
}

void StockInsight::loadCSV()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Выберите CSV файл", "", "CSV файлы (*.csv);;Все файлы (*)");

    if (filePath.isEmpty()) {
        return;
    }

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

    file.close();
    QMessageBox::information(this, "Готово", "Загружено " + QString::number(row) + " товаров!");
}