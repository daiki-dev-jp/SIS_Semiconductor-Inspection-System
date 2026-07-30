#include "CsvWriter.h"

#include <QFile>
#include <QTextStream>
#include <QDir>

bool CsvWriter::write(const QVector<MeasurementRecord>& records)
{
    m_errorString.clear();

    QString folder = "./result";

    QDir dir;
    if (!dir.mkpath(folder))
    {
        m_errorString = "保存先フォルダを作成できません。";
        return false;
    }

    QString fileName =
        folder + "/Measurement_" + records.first().deviceName +
        records.first().dateTime.toString("yyyyMMdd_HHmmss")
        + ".csv";

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        m_errorString = "CSVファイルを開けません。";
        return false;
    }

    QTextStream out(&file);

    out.setEncoding(QStringConverter::Utf8);

    //-----------------------------------
    // ヘッダー
    //-----------------------------------

    out
        << "DateTime,"
        << "DeviceName,"
        << "WaferID,"
        << "LotNo,"
        << "RecipeName,"
        << "PartNumber,"
        << "WaferType,"
        << "Operator"
        << "MeasurementLineCount,"
        << "LineNumber,"
        << "LinePosition,"
        << "AverageThickness,"
        << "UpperLimit,"
        << "LowerLimit,"
        << "Judgment,"
        << "\n";

    //-----------------------------------
    // データ
    //-----------------------------------

    for (const auto& record : records)
    {
        out
            << record.dateTime.toString("yyyy/MM/dd HH:mm:ss") << ","
            << record.deviceName << ","
            << record.waferId << ","
            << record.lotNo << ","
            << record.recipeName << ","
            << record.partNumber << ","
            << record.waferType << ","
            << record.operatorName << ","
            << record.measurementLineCount << ","
            << record.lineNumber << ","
            << record.linePosition << ","
            << record.averageThickness << ","
            << record.upperLimit << ","
            << record.lowerLimit << ","
            << record.judgment
            << "\n";
    }

    return true;
}

QString CsvWriter::errorString() const
{
    return m_errorString;
}