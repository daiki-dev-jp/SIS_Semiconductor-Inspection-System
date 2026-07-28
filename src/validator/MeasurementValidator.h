#pragma once

#include <QString>

#include "model/MeasurementInfo.h"

class MeasurementValidator {
public:
	static bool validate(
		const MeasurementInfo& info,
		QString& errorMessage
	);
};