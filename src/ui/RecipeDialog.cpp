#include "ui/RecipeDialog.h"
#include "ui_RecipeDialog.h"
#include "model/Recipe.h"
#include "repository/RecipeRepository.h"

#include <QMessageBox>
#include <QUuid>

RecipeDialog::RecipeDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::RecipeDialogClass)
{
    ui->setupUi(this);

    initializeUi();
    setupConnections();
    initializeState();
}

RecipeDialog::~RecipeDialog()
{
    delete ui;
}


//=============================================================================
// Public Method
//=============================================================================

//既存レシピ一覧を設定する
void RecipeDialog::setExistingRecipes(const QList<Recipe>& recipes) {
    m_existingRecipes = recipes;
}

//編集対象レシピを設定する
void RecipeDialog::setRecipe(const Recipe& recipe) {
    m_originalRecipe = recipe;
    displayRecipe(recipe);
    ui->deletePushButton->setVisible(true);
}

QString RecipeDialog::savedRecipeId() const {
    return recipeId;
}


//=============================================================================
// Initialization
//=============================================================================


void RecipeDialog::initializeUi()
{
    // UIの初期表示・見た目の設定

    ui->deletePushButton->setVisible(false);
}

void RecipeDialog::setupConnections()
{
    //シグナルとスロットの接続
    //システム
    connect(ui->savePushButton,
        &QPushButton::clicked,
        this,
        &RecipeDialog::onSaveClicked
    );

    connect(ui->closePushButton,
        &QPushButton::clicked,
        this,
        &RecipeDialog::onCloseClicked
    );

    connect(ui->deletePushButton,
        &QPushButton::clicked,
        this,
        &RecipeDialog::onDeleteClicked
    );
}

void RecipeDialog::initializeState()
{
    // 初期状態・初期データの設定
    initializeWaferTypeButtons();
}

//ウェーハ種類のグループ化処理
void RecipeDialog::initializeWaferTypeButtons() {
    waferTypeGroup = new QButtonGroup(this);
    waferTypeGroup->addButton(ui->pwRadioButton, 0);
    waferTypeGroup->addButton(ui->cwRadioButton, 1);
    waferTypeGroup->addButton(ui->lwRadioButton, 2);
    waferTypeGroup->addButton(ui->swRadioButton, 3);

    ui->pwRadioButton->setChecked(true);
}

//=============================================================================
// Slots
//=============================================================================


void RecipeDialog::onSaveClicked() {
    Recipe recipe = createRecipeFromUi();

    if (!validateRecipe(recipe)) {
        return;
    }

    if (!saveRecipeData(recipe)) {
        return;
    }
    
    accept();
}

void RecipeDialog::onCloseClicked() {    
    reject();
}

void RecipeDialog::onDeleteClicked() {
    RecipeRepository repository;
    if (!repository.remove(m_originalRecipe.id)) {
        QMessageBox::warning(
            this,
            "削除エラー",
            "レシピを削除できませんでした。");
        return;
    }

    accept();
}

//=============================================================================
// Private Methods
//=============================================================================

void RecipeDialog::displayRecipe(const Recipe& recipe) {
    ui->recipeNameLineEdit->setText(recipe.recipeName);
    ui->partNumberLineEdit->setText(recipe.partNumber);
    for (QAbstractButton* button : waferTypeGroup->buttons()) {
        if (button->text() == recipe.waferType) {
            button->setCheckable(true);
            break;
        }
    }
    ui->lineCountComboBox->setCurrentText(QString::number(recipe.lineCount));
    ui->upperLimitDoubleSpinBox->setValue(recipe.upperLimit);
    ui->lowerLimitDoubleSpinBox->setValue(recipe.lowerLimit);
    ui->commentLineEdit->setText(recipe.comment);
}

Recipe RecipeDialog::createRecipeFromUi() {
    Recipe recipe = m_originalRecipe;
    recipe.recipeName = ui->recipeNameLineEdit->text();
    recipe.partNumber = ui->partNumberLineEdit->text();
    QAbstractButton* button = waferTypeGroup->checkedButton();
    if (button) {
        recipe.waferType = button->text();
    }
    recipe.lineCount = ui->lineCountComboBox->currentText().toInt();
    recipe.upperLimit = ui->upperLimitDoubleSpinBox->value();
    recipe.lowerLimit = ui->lowerLimitDoubleSpinBox->value();
    recipe.comment = ui->commentLineEdit->text();
    return recipe;
}

//バリデーションチェック
bool RecipeDialog::validateRecipe(const Recipe& recipe) {
    //必須チェック
    if (recipe.recipeName.trimmed().isEmpty()) {
        QMessageBox::warning(
            this,
            "エラー",
            "レシピ名は必須項目です。");
        return false;
    }

    if (recipe.partNumber.trimmed().isEmpty()) {
        QMessageBox::warning(
            this,
            "エラー",
            "品番は必須項目です。");
        return false;
    }

    //重複チェック
    for (const Recipe& recipeTemp : m_existingRecipes) {
        //名前が違うなら次へ
        if (recipeTemp.recipeName.trimmed() != recipe.recipeName.trimmed()) {
            continue;
        }

        //編集中の自分自身なら重複ではない
        if (!m_originalRecipe.isEmpty() &&
            recipeTemp.id == m_originalRecipe.id) {
            continue;
        }

        QMessageBox::warning(
            this,
            "エラー",
            "同じレシピ名が既に存在します。");
        return false;
    }
 
    //上下限チェック
    if (recipe.upperLimit < recipe.lowerLimit) {
        QMessageBox::warning(
            this,
            "エラー",
            "上限値は下限値以上にしてください。"
        );
        return false;
    }

    return true;
}

bool RecipeDialog::saveRecipeData(Recipe& recipe) {
    RecipeRepository repository;
    if (m_originalRecipe.isEmpty()) {
        // UUIDをレシピIDとして使用。
        // UUID v4は理論上重複する可能性はあるが、
        // 通常用途では十分な一意性を持つ。
        recipe.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        //新規登録
        if (!repository.save(recipe)) {
            QMessageBox::warning(
                this,
                "保存エラー",
                "レシピを保存できませんでした。");
            return false;
        }
        recipeId = recipe.id;
    }
    else {
        //更新
        if (!repository.update(recipe)) {
            QMessageBox::warning(
                this,
                "保存エラー",
                "レシピを保存できませんでした。");
            return false;
        }
    }
    return true;
}