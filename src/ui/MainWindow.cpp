#include <QMessageBox>

#include "ui/MainWindow.h"
#include "ui_MainWindow.h"
#include "ui/RecipeDialog.h"
#include "repository/CsvWriter.h"
#include "repository/RecipeRepository.h"
#include "validator/MeasurementValidator.h"
#include "core/LogLevel.h"
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
// Protected Methods
//=============================================================================

void MainWindow::closeEvent(QCloseEvent* event)
{
    QMessageBox::StandardButton result =
        QMessageBox::question(
            this,
            "終了確認",
            "アプリケーションを終了しますか？",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

    if (result == QMessageBox::Yes)
    {
        m_logger.write(
            LogLevel::Info,
            "アプリケーションを終了しました。",
            Q_FUNC_INFO);

        event->accept();
    }
    else
    {
        event->ignore();
    }
}

//=============================================================================
// Slots
//=============================================================================

void MainWindow::onSendInfoClicked() {
    m_logger.write(LogLevel::Info, "「情報送信」ボタンを押下しました。", Q_FUNC_INFO);

    if (!sendMeasurementInfo()) {
        return;
    }

    // 情報送信
    m_stateMachine.setState(State::READY);
    updateUiByState();
    m_logger.write(LogLevel::Info, "状態を READY に遷移しました。", Q_FUNC_INFO);
}

void MainWindow::onStartMeasurementClicked() {
    m_logger.write(LogLevel::Info, "「測定開始」ボタンを押下しました。", Q_FUNC_INFO);

    if (!sendMeasurementInfo()) {
        return;
    }

    MeasurementInfo info = createMeasurementInfoFromUi();

    Recipe recipe = m_recipes[ui->recipeComboBox->currentIndex()];

    m_stateMachine.setState(State::START);
    updateUiByState();
    m_logger.write(LogLevel::Info, "状態を START に遷移しました。", Q_FUNC_INFO);

    m_logger.write(LogLevel::Info, "データの取得を開始しました。", Q_FUNC_INFO);
    QVector<MeasurementResult> results;

    if (!m_plcController.receiveMeasurementResults(recipe.lineCount, results)) {
        m_logger.write(LogLevel::Error, "PLCからのデータ取得に失敗しました。", Q_FUNC_INFO);
        m_errorType = ErrorType::MeasurementError;
        checkError();
        return;
    }
    else {
        m_logger.write(LogLevel::Info, "PLCからのデータ取得に成功しました。", Q_FUNC_INFO);
    }

    double totalAverage = calculateTotalAverage(results);

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
        m_logger.write(LogLevel::Error, "CSVの保存に失敗しました。", Q_FUNC_INFO);
        m_errorType = ErrorType::CsvError;
        checkError();
        return;
    }
    else {
        m_logger.write(LogLevel::Info, "CSVの保存に成功しました。", Q_FUNC_INFO);
    }

    m_stateMachine.setState(State::COMPLETE);
    updateUiByState();
    m_logger.write(LogLevel::Info, "状態を COMPLETE に遷移しました。", Q_FUNC_INFO);
}

void MainWindow::onResetClicked() {
    m_logger.write(LogLevel::Info, "「リセット」ボタンを押下しました。", Q_FUNC_INFO);
    clearMeasurementResult();
    m_errorType = ErrorType::None;
    m_stateMachine.setState(State::IDLE);
    updateUiByState();
    m_logger.write(LogLevel::Info, "状態を IDLE に遷移しました。", Q_FUNC_INFO);
}

void MainWindow::onAddRecipeClicked() {
    m_logger.write(LogLevel::Info, "「レシピ追加」ボタンを押下しました。", Q_FUNC_INFO);
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
    m_logger.write(LogLevel::Info, "「レシピ編集」ボタンを押下しました。", Q_FUNC_INFO);
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

    m_logger.write(
        LogLevel::Info,
        QString("レシピ「%1」を選択しました。")
        .arg(ui->recipeComboBox->currentText()),
        Q_FUNC_INFO);
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
    m_logger.write(LogLevel::Info, "状態を IDLE に遷移しました。", Q_FUNC_INFO);
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

        ui->sendInfoPushButton->setEnabled(true);
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

bool MainWindow::sendMeasurementInfo() {
    MeasurementInfo info = createMeasurementInfoFromUi();
    QString error;
    if (!MeasurementValidator::validate(info, error))
    {
        m_logger.write(LogLevel::Warning, error, Q_FUNC_INFO);
        QMessageBox::warning(this,
            "入力エラー",
            error);
        return false;
    }

    m_logger.write(LogLevel::Info, "PLCへ情報の送信を開始しました。", Q_FUNC_INFO);

    //PLCへ情報送信（シミュレーション）
    if (!m_plcController.sendMeasurementInfo(info)) {
        m_logger.write(LogLevel::Error, "PLCへ情報の送信が失敗しました。", Q_FUNC_INFO);
		m_errorType = ErrorType::PlcError;
		checkError();
        return false;
    }
    else {
        m_logger.write(LogLevel::Info, "PLCへ情報の送信が成功しました。", Q_FUNC_INFO);
    }

    return true;
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
	QString errorMessage;

    switch (m_errorType)
    {
    case ErrorType::PlcError:
        errorMessage = "PLC通信エラー";
        ui->errorPlainTextEdit->setPlainText(errorMessage);
        break;

    case ErrorType::CsvError:
        errorMessage = "CSV保存エラー";
        ui->errorPlainTextEdit->setPlainText(errorMessage);
        break;

    case ErrorType::MeasurementError:
        errorMessage = "測定エラー";
        ui->errorPlainTextEdit->setPlainText(errorMessage);
        break;
    }

    m_stateMachine.setState(State::ERROR);
    updateUiByState();
    QMessageBox::warning(this,
        "エラー",
        errorMessage);

    m_logger.write(LogLevel::Info, "状態を ERROR に遷移しました。", Q_FUNC_INFO);
    return true;
}
