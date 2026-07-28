#pragma once

#include <QtWidgets/QMainWindow>

#include "model/Recipe.h"
#include "model/MeasurementInfo.h"
#include "core/StateMachine.h"
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

private:
    // Initialization
    void initializeUi();
    void setupConnections();
    void initializeState();

    // Slots
    void onSendInfoClicked();
    void onStartMeasurementClicked();
    void onResetClicked();
    void onAddRecipeClicked();
    void onEditRecipeClicked();
    void onRecipeChanged(int index);

    // Private Methods
    void loadRecipes();
    void selectRecipe(const QString& recipeId);
    void updateRecipeInfo();
    MeasurementInfo createMeasurementInfoFromUi() const;
    StateMachine m_stateMachine;
    PlcController m_plcController;

private:
    Ui::MainWindowClass*ui;
    QList<Recipe> m_recipes;
};

