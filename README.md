# StockInsight

Проект для анализа складских запасов.

Участники:
- Каусов Д.А. (бэкенд)
- Никишина Д.В. (фронтенд)

# Запуск программы
cd ~/programming/StockInsight
qmake6 StockInsight.pro
make -j$(nproc)
./StockInsight

# Тесты
cd ~/programming/StockInsight
qmake6 "CONFIG+=testcase" StockInsight.pro
make -j$(nproc)
./StockInsightTests
