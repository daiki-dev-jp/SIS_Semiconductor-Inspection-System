#include "controller/PlcController.h"

#include <QRandomGenerator>

//=============================================================================
// Public Methods
//=============================================================================

bool PlcController::sendMeasurementInfo(const MeasurementInfo& info) {
    Q_UNUSED(info);

    //本来はPLCへ通信
    //ポートフォリオでは通信成功として扱う

    return true;
}

MeasurementResult PlcController::receiveMeasurementResult() {
    MeasurementResult result;

    //ランダム測定値(700.00～760.00μm)
    double min = 700.0;
    double max = 760.0;

    result.averageThickness =
        min + QRandomGenerator::global()->generateDouble()
        * (max - min);

    return result;
}