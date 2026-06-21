#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_StockInsight.h"

class StockInsight : public QMainWindow
{
    Q_OBJECT

public:
    StockInsight(QWidget *parent = nullptr);
    ~StockInsight();

private:
    Ui::StockInsightClass ui;
};

