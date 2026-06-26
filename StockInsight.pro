QT       += core gui widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

TARGET = StockInsight
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/models/Product.cpp \
    src/models/Analytics.cpp \
    src/services/CsvParser.cpp \
    src/services/DataMerger.cpp

HEADERS += \
    src/models/Product.h \
    src/models/Analytics.h \
    src/services/CsvParser.h \
    src/services/DataMerger.h