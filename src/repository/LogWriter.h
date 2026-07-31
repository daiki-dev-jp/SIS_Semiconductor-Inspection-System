#pragma once

#include <QString>

#include "core/LogLevel.h"

class LogWriter
{
public:
    bool write(LogLevel level, const QString& message, const char* function);

    void deleteOldLogs(int keepDays = 28);

private:
    QString logFolder() const;
    QString logFilePath() const;
    QString levelToString(LogLevel level) const;
    QString formatFunctionName(const char* function);
};