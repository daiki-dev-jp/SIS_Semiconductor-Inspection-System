#pragma once
#include <QVector>

#include "model/MeasurementInfo.h"
#include "model/MeasurementResult.h"

class PlcController
{
public:
	bool sendMeasurementInfo(const MeasurementInfo& info);

	bool receiveMeasurementResults(int lineCount, QVector<MeasurementResult>& results);

	bool receiveMeasurementResult(MeasurementResult& result);
};