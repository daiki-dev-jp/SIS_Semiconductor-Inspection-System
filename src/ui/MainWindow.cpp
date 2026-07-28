#include <QMessageBox>

#include "ui/MainWindow.h"
#include "ui_MainWindow.h"
#include "ui/RecipeDialog.h"
#include "repository/RecipeRepository.h"
#include "validator/MeasurementValidator.h"
#include "core/State.h"
#include "core/StateMachine.h"
#include "model/MeasurementResult.h"
#include "model/CsvRecord.h"

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

void MainWindow::initializeState()
{
    // 初期状態
    m_stateMachine.setState(State::IDLE);
}

//=============================================================================
// Slots
//=============================================================================

void MainWindow::onSendInfoClicked() {
    MeasurementInfo info = createMeasurementInfoFromUi();

    QString error;

    if (!MeasurementValidator::validate(info, error))
    {
        QMessageBox::warning(this,
            "入力エラー",
            error);
        return;
    }

    //PLCへ情報送信（シミュレーション）
    if (!m_plcController.sendMeasurementInfo(info)) {
        QMessageBox::warning(this,
            "通信エラー",
            "PLCとの通信に失敗しました。");
        return;
    }

    // 情報送信
    m_stateMachine.setState(State::READY);
}

void MainWindow::onStartMeasurementClicked() {
    // 測定開始
    m_stateMachine.setState(State::START);

    //--------------------------------------------------------
    // メイン画面・レシピから取得
    //--------------------------------------------------------

    MeasurementInfo info = createMeasurementInfoFromUi();

    Recipe recipe = m_recipes[ui->recipeComboBox->currentIndex()];

    int lineCount = recipe.lineCount;

    //--------------------------------------------------------
    // PLCから測定結果取得
    //--------------------------------------------------------

    QVector<MeasurementResult> results;

    for (int lineNo = 1; lineNo <= lineCount; ++lineNo)
    {
        results.append(
            m_plcController.receiveMeasurementResult());
    }

    //--------------------------------------------------------
    // PC側で測定完了時刻取得
    //--------------------------------------------------------

    QDateTime measurementTime =
        QDateTime::currentDateTime();

    //--------------------------------------------------------
    // 全体平均膜厚計算
    //--------------------------------------------------------

    double totalAverage = 0.0;

    for (const auto& result : results)
    {
        totalAverage += result.averageThickness;
    }

    totalAverage /= results.size();

    // TODO://CSV出力で利用
    //--------------------------------------------------------
    // CSVデータ生成
    //--------------------------------------------------------

    QVector<CsvRecord> records;

    int step = 200 / (lineCount - 1);

    for (int i = 0; i < results.size(); ++i)
    {
        CsvRecord record;

        record.measurementTime = measurementTime;

        record.deviceName = info.equipmentNo;
        record.waferId = info.waferId;
        record.lotNo = info.lotNo;
        record.recipeName = recipe.recipeName;
        record.partNumber = recipe.partNumber;
        record.waferType = recipe.waferType;
        record.operatorName = info.operatorName;

        record.measurementLineCount = lineCount;

        //---------------------------------------
        // PC側で計算
        //---------------------------------------

        record.lineNumber = i + 1;

        record.linePosition = i * step;

        record.averageThickness =
            results[i].averageThickness;

        record.totalAverageThickness =
            totalAverage;

        record.upperLimit = recipe.upperLimit;

        record.lowerLimit = recipe.lowerLimit;

        //---------------------------------------
        // 判定
        //---------------------------------------

        if (record.averageThickness >= record.lowerLimit &&
            record.averageThickness <= record.upperLimit)
        {
            record.judgment = "OK";
        }
        else
        {
            record.judgment = "NG";
        }

        records.append(record);
    }

    //--------------------------------------------------------
    // CSV保存
    //--------------------------------------------------------

     //CsvWriter writer;
     //writer.write(records);

    m_stateMachine.setState(State::COMPLETE);
}

void MainWindow::onResetClicked() {
    // リセット
    m_stateMachine.setState(State::IDLE);
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

MeasurementInfo MainWindow::createMeasurementInfoFromUi() const {
    MeasurementInfo info;

    info.waferId = ui->waferIdLineEdit->text();
    info.lotNo = ui->lotNoLineEdit->text();
    info.operatorName = ui->operatorNameLineEdit->text();
    info.equipmentNo = ui->equipmentNoComboBox->currentText();
    info.recipe = ui->recipeComboBox->currentText();
    
    return info;
}

