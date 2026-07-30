#pragma once
#include <QDateTime>
#include <QString>

struct MeasurementRecord {
    QDateTime dateTime;

    QString deviceName;
    QString waferId;
    QString lotNo;
    QString recipeName;
    QString partNumber;
    QString waferType;
    QString operatorName;

    int measurementLineCount;
    int lineNumber;
    int linePosition;

    double averageThickness;
    double totalAverageThickness;

    double upperLimit;
    double lowerLimit;

    QString judgment;
};