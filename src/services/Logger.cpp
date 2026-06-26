#include "Logger.h"
#include <QDebug>

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

Logger::~Logger() {
    if (m_file.isOpen()) m_file.close();
}

void Logger::init(const QString &filePath) {
    if (m_file.isOpen()) m_file.close();
    
    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::Append | QIODevice::Text)) {
    qWarning() << "Не удалось открыть лог-файл:" << filePath;
}
    
    QTextStream out(&m_file);
    out.setEncoding(QStringConverter::Utf8);
    out << "Лог запущен" 
        << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") 
        << "\n";
    out.flush();
}

void Logger::log(const QString &message, Level level) {
    QMutexLocker locker(&m_mutex);
    
    QString entry = QString("[%1] [%2] %3")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
        .arg(levelToString(level))
        .arg(message);
    
    // В консоль
    qDebug() << entry;
    
    // В файл
    if (m_file.isOpen()) {
        QTextStream out(&m_file);
        out << entry << "\n";
        out.flush();
    }
}

QString Logger::levelToString(Level level) {
    switch (level) {
        case INFO:    return "INFO";
        case WARNING: return "WARN";
        case ERROR:   return "ERROR";
    }
    return "?";
}