#pragma once
#include <QMainWindow>
#include <QTableWidget>

class StockInsight : public QMainWindow
{
    Q_OBJECT

public:
    StockInsight(QWidget* parent = nullptr);
    ~StockInsight() {}

private slots:
    void loadCSV();

private:
    QTableWidget* table;
};