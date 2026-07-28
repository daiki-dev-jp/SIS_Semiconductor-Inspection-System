#include <QString>
#include <QRegularExpression>

#include "validator/MeasurementValidator.h"

namespace {
    //半角英数字とハイフンのみ許可
    static const QRegularExpression idRegex("^[A-Za-z0-9-]+$");

    //半角英数字のみ許可
    static const QRegularExpression operatorRegex("^[A-Za-z0-9]+$");
}

bool MeasurementValidator::validate(const MeasurementInfo& info,
	QString& errorMessage) {
    //必須項目
    if (info.waferId.trimmed().isEmpty()) {
        errorMessage = "ウェーハIDを入力してください。";
        return false;
    }

    if (info.lotNo.trimmed().isEmpty()) {
        errorMessage = "Lot番号を入力してください。";
        return false;
    }

    if (info.operatorName.trimmed().isEmpty()) {
        errorMessage = "作業者を入力してください。";
        return false;
    }

    if (!idRegex.match(info.waferId).hasMatch()) {
        errorMessage = "ウェーハIDは半角英数字とハイフン(-)のみ入力できます。";
        return false;
    }

    if (!idRegex.match(info.lotNo).hasMatch()) {
        errorMessage = "Lot番号は半角英数字とハイフン(-)のみ入力できます。";
        return false;
    }

    if (!operatorRegex.match(info.operatorName).hasMatch()) {
        errorMessage = "作業者は半角英数字のみ入力できます。";
        return false;
    }

    return true;
}
