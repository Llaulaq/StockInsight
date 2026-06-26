#ifndef TEST_ANALYTICS_ENGINE_H
#define TEST_ANALYTICS_ENGINE_H

#include <QtTest>
#include "../src/services/AnalyticsEngine.h"

class TestAnalyticsEngine : public QObject {
    Q_OBJECT

private slots:
    void testProfit();
    void testDeficit();
    void testStale();
};

#endif