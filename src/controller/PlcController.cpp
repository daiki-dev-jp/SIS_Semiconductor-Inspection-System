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

    //ランダム測定値(99.30～100.70μm)
    double min = 99.3;
    double max = 100.7;

    result.averageThickness =
        min + QRandomGenerator::global()->generateDouble()
        * (max - min);

    return result;
}