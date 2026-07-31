#pragma once

#include <QCloseEvent>
#include <QtWidgets/QMainWindow>

#include "model/Recipe.h"
#include "model/MeasurementInfo.h"
#include "model/MeasurementRecord.h"
#include "core/ErrorType.h"
#include "core/StateMachine.h"
#include "repository/LogWriter.h"
#include "controller/PlcController.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindowClass;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    // Slots
    void onSendInfoClicked();
    void onStartMeasurementClicked();
    void onResetClicked();
    void onAddRecipeClicked();
    void onEditRecipeClicked();
    void onRecipeChanged(int index);

    // Initialization
    void initializeUi();
    void setupConnections();
    void initializeState();

    // Private Methods
    void loadRecipes();
    void selectRecipe(const QString& recipeId);
    void updateRecipeInfo();
    void updateUiByState();
    void clearMeasurementResult();
    MeasurementInfo createMeasurementInfoFromUi() const;
    bool sendMeasurementInfo();
    StateMachine m_stateMachine;
    PlcController m_plcController;

    QVector<MeasurementResult> receiveMeasurementResults(int lineCount);
    double calculateTotalAverage(const QVector<MeasurementResult>& results);
    QVector<MeasurementRecord> createMeasurementRecords(
            const MeasurementInfo& info,
            const Recipe& recipe,
            const QVector<MeasurementResult>& results,
            const QDateTime& dateTime,
            const double& totalAverage
        );

    void updateResultTable(const QVector<MeasurementRecord>& records);
    ErrorType m_errorType = ErrorType::None;
    bool checkError();
    LogWriter m_logger;

private:
    Ui::MainWindowClass*ui;
    QList<Recipe> m_recipes;
};

