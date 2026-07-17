#pragma once

#include <QtWidgets/QMainWindow>

#include "model/Recipe.h"

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

private:
    Ui::MainWindowClass*ui;
    QList<Recipe> m_recipes;
};

