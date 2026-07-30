#include <QMessageBox>

#include "ui/MainWindow.h"
#include "ui_MainWindow.h"
#include "ui/RecipeDialog.h"
#include "repository/CsvWriter.h"
#include "repository/RecipeRepository.h"
#include "validator/MeasurementValidator.h"
#include "core/State.h"
#include "core/StateMachine.h"
#include "model/MeasurementResult.h"
#include "model/MeasurementRecord.h"

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
    ui->resultTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->errorPlainTextEdit->setFocusPolicy(Qt::NoFocus);

    ui->resultTableWidget->setColumnWidth(0, 120); // 測定位置
    ui->resultTableWidget->setColumnWidth(1, 120); // 平均膜厚
    ui->resultTableWidget->setColumnWidth(2, 80);  // 判定
    ui->resultTableWidget->setAlternatingRowColors(true);
    ui->resultTableWidget->setStyleSheet(
        "QTableWidget {"
        "    alternate-background-color: rgb(240,240,240);"
        "    background-color: white;"
        "}");

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
    updateUiByState();
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
    updateUiByState();
}

void MainWindow::onStartMeasurementClicked() {
    // 測定開始
    m_stateMachine.setState(State::START);
    updateUiByState();

    MeasurementInfo info = createMeasurementInfoFromUi();

    Recipe recipe = m_recipes[ui->recipeComboBox->currentIndex()];

    QVector<MeasurementResult> results =
        receiveMeasurementResults(recipe.lineCount);

    if (checkError()) {
        return;
    }

    double totalAverage = calculateTotalAverage(results);

    // TODO://CSV出力でも利用
    QVector<MeasurementRecord> records 
        = createMeasurementRecords(
            info,
            recipe,
            results,
            QDateTime::currentDateTime(),
            totalAverage
            );

    updateResultTable(records);

    CsvWriter writer;

    if (!writer.write(records))
    {
        //ログに以下を残す
        //writer.errorString()
        m_errorType = ErrorType::CsvError;
        checkError();
        return;
    }

    m_stateMachine.setState(State::COMPLETE);
    updateUiByState();
}

void MainWindow::onResetClicked() {
    // リセット
    clearMeasurementResult();
    m_errorType = ErrorType::None;
    m_stateMachine.setState(State::IDLE);
    updateUiByState();
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

void MainWindow::updateUiByState()
{
    switch (m_stateMachine.currentState())
    {
    case State::IDLE:

        ui->sendInfoPushButton->setEnabled(true);
        ui->startMeasurementPushButton->setEnabled(false);
        ui->resetPushButton->setEnabled(false);

        break;

    case State::READY:

        ui->sendInfoPushButton->setEnabled(false);
        ui->startMeasurementPushButton->setEnabled(true);
        ui->resetPushButton->setEnabled(false);

        break;

    case State::START:

        ui->sendInfoPushButton->setEnabled(false);
        ui->startMeasurementPushButton->setEnabled(false);
        ui->resetPushButton->setEnabled(false);

        break;

    case State::COMPLETE:
    case State::ERROR:

        ui->sendInfoPushButton->setEnabled(false);
        ui->startMeasurementPushButton->setEnabled(false);
        ui->resetPushButton->setEnabled(true);

        break;
    }
}

void MainWindow::clearMeasurementResult() {
    ui->resultTableWidget->clearContents();
    ui->resultTableWidget->setRowCount(0);

    ui->errorPlainTextEdit->clear();
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

// PLCから測定結果取得
QVector<MeasurementResult> MainWindow::receiveMeasurementResults(int lineCount) {
    QVector<MeasurementResult> results;

    for (int lineNo = 0; lineNo < lineCount; ++lineNo)
    {
        results.append(
            m_plcController.receiveMeasurementResult());
    }

    return results;
}

// 全体平均膜厚計算
double MainWindow::calculateTotalAverage(const QVector<MeasurementResult>& results) {
    double total = 0.0;

    for (const auto& result : results)
    {
        total += result.averageThickness;
    }

    return total /= results.size();   
}

// データ生成
QVector<MeasurementRecord> MainWindow::createMeasurementRecords(
    const MeasurementInfo& info,
    const Recipe& recipe,
    const QVector<MeasurementResult>& results,
    const QDateTime& dateTime,
    const double& totalAverage
) {
    QVector<MeasurementRecord> records;
    int step = 200 / (recipe.lineCount - 1);//0割はない

    for (int i = 0; i < results.size(); ++i)
    {
        MeasurementRecord record;

        record.dateTime = dateTime;

        record.deviceName = info.equipmentNo;
        record.waferId = info.waferId;
        record.lotNo = info.lotNo;
        record.recipeName = recipe.recipeName;
        record.partNumber = recipe.partNumber;
        record.waferType = recipe.waferType;
        record.operatorName = info.operatorName;

        record.measurementLineCount = recipe.lineCount;

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
    return records;
}


// 測定結果表示
void MainWindow::updateResultTable(const QVector<MeasurementRecord>& records) {
    ui->resultTableWidget->clearContents();
    ui->resultTableWidget->setRowCount(records.size());

    for (int row = 0; row < records.size(); ++row)
    {
        const MeasurementRecord& record = records[row];

        // ===== 測定位置 =====
        auto* lineItem = new QTableWidgetItem(
            QString::number(record.linePosition));
        lineItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

        // ===== 平均膜厚 =====
        auto* thicknessItem = new QTableWidgetItem(
            QString::number(record.averageThickness, 'f', 2));
        thicknessItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);

        // ===== 判定 =====
        auto* judgmentItem = new QTableWidgetItem(record.judgment);
        judgmentItem->setTextAlignment(Qt::AlignCenter);
        if (record.judgment == "OK")
        {
            judgmentItem->setForeground(QBrush(Qt::darkGreen));
        }
        else if (record.judgment == "NG")
        {
            judgmentItem->setForeground(QBrush(Qt::red));
        }

        ui->resultTableWidget->setItem(row, 0, lineItem);
        ui->resultTableWidget->setItem(row, 1, thicknessItem);
        ui->resultTableWidget->setItem(row, 2, judgmentItem);
    }
}

//エラーチェック
bool MainWindow::checkError()
{
    if (m_errorType == ErrorType::None)
        return false;

    switch (m_errorType)
    {
    case ErrorType::PlcError:
        ui->errorPlainTextEdit->setPlainText("PLC通信エラー");
        break;

    case ErrorType::CsvError:
        ui->errorPlainTextEdit->setPlainText("CSV保存エラー");
        break;

    case ErrorType::MeasurementError:
        ui->errorPlainTextEdit->setPlainText("測定エラー");
        break;
    }

    m_stateMachine.setState(State::ERROR);
    updateUiByState();

    return true;
}
