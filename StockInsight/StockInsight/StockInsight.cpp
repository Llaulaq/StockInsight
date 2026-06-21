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


// КОНСТРУКТОР
StockInsight::StockInsight(QWidget* parent)
    : QMainWindow(parent)
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

    QLineEdit* searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("🔍 Поиск по товарам...");

    buttonLayout->addWidget(loadBtn);
    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(exportBtn);
    buttonLayout->addWidget(clearBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(searchEdit);

    mainLayout->addLayout(buttonLayout);

    // --- Таблица ---
    QLabel* tableLabel = new QLabel("📋 Список товаров");
    mainLayout->addWidget(tableLabel);

    table = new QTableWidget(0, 7);
    QStringList headers = { "Товар", "Категория", "Кол-во", "Цена зак.", "Цена прод.", "Дней", "Прибыль" };
    table->setHorizontalHeaderLabels(headers);
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
    connect(clearBtn, &QPushButton::clicked, this, &StockInsight::clearTable);
}

// ЗАГРУЗКА CSV
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
    }
    else {
        loadBtn->setEnabled(true);
        saveBtn->setEnabled(true);
        exportBtn->setEnabled(true);
    }
}

// ОЧИСТКА ТАБЛИЦЫ (только для админа)
void StockInsight::clearTable()
{
    if (currentRole != "admin") {
        QMessageBox::warning(this, "Доступ запрещён",
            "Только администратор может очищать таблицу!");
        return;
    }

    table->setRowCount(0);
    QMessageBox::information(this, "Готово", "Таблица очищена!");
}