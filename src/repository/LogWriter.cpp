#include "LogWriter.h"

#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

//=============================================================================
// Public Methods
//=============================================================================

bool LogWriter::write(LogLevel level,
    const QString& message,
    const char* function)
{
    //--------------------------------------------------
    // logフォルダ作成
    //--------------------------------------------------

    QDir dir;

    if (!dir.mkpath(logFolder()))
    {
        //エラー
    }

    QFile file(logFilePath());

    bool newFile = !file.exists();

    if (!file.open(QIODevice::Append | QIODevice::Text))
    {
        return false;
    }

    QTextStream out(&file);

    out.setEncoding(QStringConverter::Utf8);

    //--------------------------------------------------
    // ヘッダー
    //--------------------------------------------------

    if (newFile)
    {
        out
            << "DateTime,"
            << "Level,"
            << "Message\n";
    }

    //--------------------------------------------------
    // データ
    //--------------------------------------------------

    out
        << QDateTime::currentDateTime()
        .toString("yyyy/MM/dd HH:mm:ss")
        << ","
        << levelToString(level)
        << ","
        << formatFunctionName(function)
        << ":"
        << message
        << "\n";

    return true;
}

void LogWriter::deleteOldLogs(int keepDays)
{
    QDir dir(logFolder());

    if (!dir.exists())
    {
        return;
    }

    QFileInfoList files =
        dir.entryInfoList(
            QStringList() << "Log_*.csv",
            QDir::Files);

    QDate limitDate =
        QDate::currentDate().addDays(-keepDays);

    for (const QFileInfo& fileInfo : files)
    {
        QString fileName = fileInfo.baseName();

        // Log_20260730
        QString dateText =
            fileName.mid(4);

        QDate date =
            QDate::fromString(
                dateText,
                "yyyyMMdd");

        if (date.isValid() &&
            date < limitDate)
        {
            QFile::remove(fileInfo.absoluteFilePath());
        }
    }
}

//=============================================================================
// Private Methods
//=============================================================================

QString LogWriter::logFolder() const
{
    return "./log";
}

QString LogWriter::logFilePath() const
{
    return logFolder() +
        "/Log_" +
        QDate::currentDate().toString("yyyyMMdd") +
        ".csv";
}

QString LogWriter::levelToString(LogLevel level) const
{
    switch (level)
    {
    case LogLevel::Info:
        return "INFO";

    case LogLevel::Warning:
        return "WARNING";

    case LogLevel::Error:
        return "ERROR";
    }

    return "";
}

QString LogWriter::formatFunctionName(const char* function)
{
    QString name = function;

    // 引数を削除
    int pos = name.indexOf('(');
    if (pos >= 0)
        name = name.left(pos);

    // 最後のスペース以降を取得
    pos = name.lastIndexOf(' ');
    if (pos >= 0)
        name = name.mid(pos + 1);

    return name;
}