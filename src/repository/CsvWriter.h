#pragma once

#include <QVector>
#include <QString>

#include "model/MeasurementRecord.h"

class CsvWriter
{
public:
    bool write(const QVector<MeasurementRecord>& records);

    QString errorString() const;

private:
    QString m_errorString;
};