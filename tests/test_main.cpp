#include <QtTest>
#include "test_csv_parser.h"
#include "test_analytics_engine.h"

int main(int argc, char *argv[])
{
    int status = 0;
    
    TestCsvParser tc;
    status |= QTest::qExec(&tc, argc, argv);
    
    TestAnalyticsEngine ta;
    status |= QTest::qExec(&ta, argc, argv);
    
    return status;
}