#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

class Logger {
public:
    enum Level { INFO, WARNING, ERROR };

    static Logger& instance();
    void init(const QString &filePath);
    void log(const QString &message, Level level = INFO);

private:
    Logger() = default;
    ~Logger();
    
    QFile m_file;
    QMutex m_mutex;
    QString levelToString(Level level);
};

#define LOG_INFO(msg)    Logger::instance().log(msg, Logger::INFO)
#define LOG_WARNING(msg) Logger::instance().log(msg, Logger::WARNING)
#define LOG_ERROR(msg)   Logger::instance().log(msg, Logger::ERROR)

#endif 