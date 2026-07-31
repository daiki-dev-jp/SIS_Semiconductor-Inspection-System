#pragma once

#include <QDialog>
#include <QButtonGroup>

#include "model/Recipe.h"
#include "repository/LogWriter.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class RecipeDialogClass;
}
QT_END_NAMESPACE

class RecipeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RecipeDialog(QWidget* parent = nullptr);
    ~RecipeDialog();

public:
    void setExistingRecipes(const QList<Recipe>& recipes);
    void setRecipe(const Recipe& recipe);
    QString savedRecipeId() const;

private:
    // Slots
    void onSaveClicked();
    void onCloseClicked();
    void onDeleteClicked();

    // Initialization
    void initializeUi();
    void setupConnections();
    void initializeState();
    void initializeWaferTypeButtons();

    // Private Methods
    void displayRecipe(const Recipe& recipe);
    Recipe createRecipeFromUi();
    bool saveRecipeData(Recipe& recipe);
    LogWriter m_logger;

private:
    // UI
    Ui::RecipeDialogClass* ui;
    // RadioButton grouping
    QButtonGroup* waferTypeGroup;

    // Data
    QList<Recipe> m_existingRecipes;
    Recipe m_originalRecipe;

    QString recipeId;
};