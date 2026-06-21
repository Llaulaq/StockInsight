/********************************************************************************
** Form generated from reading UI file 'StockInsight.ui'
**
** Created by: Qt User Interface Compiler version 6.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STOCKINSIGHT_H
#define UI_STOCKINSIGHT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_StockInsightClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *StockInsightClass)
    {
        if (StockInsightClass->objectName().isEmpty())
            StockInsightClass->setObjectName("StockInsightClass");
        StockInsightClass->resize(600, 400);
        menuBar = new QMenuBar(StockInsightClass);
        menuBar->setObjectName("menuBar");
        StockInsightClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(StockInsightClass);
        mainToolBar->setObjectName("mainToolBar");
        StockInsightClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(StockInsightClass);
        centralWidget->setObjectName("centralWidget");
        StockInsightClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(StockInsightClass);
        statusBar->setObjectName("statusBar");
        StockInsightClass->setStatusBar(statusBar);

        retranslateUi(StockInsightClass);

        QMetaObject::connectSlotsByName(StockInsightClass);
    } // setupUi

    void retranslateUi(QMainWindow *StockInsightClass)
    {
        StockInsightClass->setWindowTitle(QCoreApplication::translate("StockInsightClass", "StockInsight", nullptr));
    } // retranslateUi

};

namespace Ui {
    class StockInsightClass: public Ui_StockInsightClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STOCKINSIGHT_H
