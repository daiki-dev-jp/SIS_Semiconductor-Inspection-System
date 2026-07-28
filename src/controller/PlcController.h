#pragma once

#include "model/MeasurementInfo.h"
#include "model/MeasurementResult.h"

class PlcController
{
public:
	bool sendMeasurementInfo(const MeasurementInfo& info);

	MeasurementResult receiveMeasurementResult();
};