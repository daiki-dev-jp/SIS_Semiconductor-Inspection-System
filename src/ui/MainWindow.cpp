#include "ui/MainWindow.h"
#include "ui_MainWindow.h"
#include "ui/RecipeDialog.h"
#include "repository/RecipeRepository.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowClass)
{
    ui->setupUi(this);
    setFixedSize(size());

    initializeUi();
    setupConnections();
    initializeState();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//=============================================================================
// Initialization
//=============================================================================


void MainWindow::initializeUi()
{
    // 初期表示設定
    ui->sendInfoPushButton->setEnabled(true);

    loadRecipes();

    updateRecipeInfo();
}

void MainWindow::setupConnections()
{
    //システム
    connect(ui->sendInfoPushButton,
        &QPushButton::clicked,
        this,
        &MainWindow::onSendInfoClicked
    );

    connect(ui->startMeasurementPushButton,
        &QPushButton::clicked,
        this,
        &MainWindow::onStartMeasurementClicked
    );

    connect(ui->resetPushButton,
        &QPushButton::clicked,
        this,
        &MainWindow::onResetClicked
    );

    //レシピ
    connect(ui->addRecipeButton,
        &QPushButton::clicked,
        this,
        &MainWindow::onAddRecipeClicked
    );

    connect(ui->editRecipeButton,
        &QPushButton::clicked,
        this,
        &MainWindow::onEditRecipeClicked
    );

    connect(ui->recipeComboBox,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &MainWindow::onRecipeChanged
    );
}

// Reserved for future initialization.
void MainWindow::initializeState()
{
    // 初期状態
}

//=============================================================================
// Slots
//=============================================================================

void MainWindow::onSendInfoClicked() {
    qDebug() << "Start Measurement";
}

void MainWindow::onStartMeasurementClicked() {

}

void MainWindow::onResetClicked() {

}

void MainWindow::onAddRecipeClicked() {
    RecipeDialog dialog(this);
    dialog.setExistingRecipes(m_recipes);

    //保存された場合のみレシピ情報を更新
    if (dialog.exec() == QDialog::Accepted) {
        QString savedRecipeId = dialog.savedRecipeId();
        loadRecipes();
        selectRecipe(savedRecipeId);

        updateRecipeInfo();
    }
}

void MainWindow::onEditRecipeClicked() {
    int index = ui->recipeComboBox->currentIndex();

    if (index < 0 || index >= m_recipes.size()) {
        return;
    }
    QString currentRecipeId =
        ui->recipeComboBox->currentData().toString();

    RecipeDialog dialog(this);
    dialog.setExistingRecipes(m_recipes);
    dialog.setRecipe(m_recipes[index]);

    if (dialog.exec() == QDialog::Accepted) {
        loadRecipes();
    }

    selectRecipe(currentRecipeId);

    updateRecipeInfo();
}

void MainWindow::onRecipeChanged(int /*index*/) {
    updateRecipeInfo();
}

//=============================================================================
// Private Methods
//=============================================================================

//レシピ情報を読み込み、コンボボックスを更新
void MainWindow::loadRecipes() {
    RecipeRepository repository;
    m_recipes = repository.loadAll();

    //コンボボックス更新中はシグナルを一時停止
    ui->recipeComboBox->blockSignals(true);

    ui->recipeComboBox->clear();

    for (const Recipe& recipe : m_recipes) {
        ui->recipeComboBox->addItem(recipe.recipeName, recipe.id);
    }
    ui->recipeComboBox->blockSignals(false);
}

void MainWindow::selectRecipe(const QString& recipeId) {
    for (int i = 0; i < ui->recipeComboBox->count(); ++i)
    {
        if (ui->recipeComboBox->itemData(i).toString() == recipeId)
        {
            ui->recipeComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void MainWindow::updateRecipeInfo() {
    int index = ui->recipeComboBox->currentIndex();

    if (index < 0 || index >= m_recipes.size()) {
        ui->recipeInfoLabel->clear();
        return;
    }

    ui->recipeInfoLabel->setText(
        m_recipes[index].partNumber + " / " + m_recipes[index].waferType);
}