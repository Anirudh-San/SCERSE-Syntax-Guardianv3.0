#include "MainWindow.hpp"
#include "CodeEditor.hpp"
#include "SyntaxHighlighter.hpp"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QTableWidgetItem>
#include <QHeaderView>
#include "c_error_detector.cpp"

namespace SCERSE {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , codeEditor(nullptr)
    , errorTable(nullptr)
    , suggestionsList(nullptr)
    , mainSplitter(nullptr)
    , analyzeTimer(nullptr)
    , statusLabel(nullptr)
    , lineColLabel(nullptr)
    , errorCountLabel(nullptr)
    , isModified(false)
{
    qDebug() << "=== MainWindow Constructor Starting ===";
    
    // ===== Central Widget Setup =====
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    
    // Create main vertical splitter
    mainSplitter = new QSplitter(Qt::Vertical, central);
    
    // Create components
    codeEditor = new CodeEditor(mainSplitter);
    errorTable = new QTableWidget(mainSplitter);
    suggestionsList = new QListWidget(mainSplitter);
    
    qDebug() << "Components created";
    
    // Setup error table
    setupErrorTable();
    
    // Configure suggestions list
    suggestionsList->setWindowTitle("Suggestions");
    suggestionsList->setMaximumHeight(150);
    
    // Add widgets to splitter
    mainSplitter->addWidget(codeEditor);
    mainSplitter->addWidget(errorTable);
    mainSplitter->addWidget(suggestionsList);
    
    // Set stretch factors (code editor gets most space)
    mainSplitter->setStretchFactor(0, 5);
    mainSplitter->setStretchFactor(1, 3);
    mainSplitter->setStretchFactor(2, 1);
    
    qDebug() << "Splitter configured";
    
    // Layout
    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->addWidget(mainSplitter);
    layout->setContentsMargins(0, 0, 0, 0);
    central->setLayout(layout);
    
    // ===== Menus and Status Bar =====
    createMenus();
    createStatusBar();
    
    qDebug() << "Menus and status bar created";
    
    // ===== Timer Setup =====
    analyzeTimer = new QTimer(this);
    analyzeTimer->setSingleShot(true);
    analyzeTimer->setInterval(500);  // 500ms debounce
    
    // ===== Connections =====
    setupConnections();
    
    qDebug() << "Connections established";
    
    // ===== Window Setup =====
    setWindowTitle("SCERSE - C Syntax Guardian");
    resize(1400, 900);
    
    // Load sample code
    if (codeEditor) {
        codeEditor->setPlainText(
            "#include <stdio.h>\n\n"
            "int main() {\n"
            "    int x = 10;\n"
            "    int y = 20;\n"
            "    \n"
            "    printf(\"Sum: %d\\n\", x + y);\n"
            "    return 0;\n"
            "}\n"
        );
    }
    
    statusBar()->showMessage("Ready");
    qDebug() << "=== MainWindow Constructor Complete ===";
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow destructor";
}

void MainWindow::setupErrorTable()
{
    qDebug() << "Setting up error table";
    
    errorTable->setColumnCount(3);
    errorTable->setHorizontalHeaderLabels({"Line", "Column", "Message"});
    
    QHeaderView *header = errorTable->horizontalHeader();
    header->setStretchLastSection(true);
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    
    errorTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    errorTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    errorTable->setAlternatingRowColors(true);
}

void MainWindow::createMenus()
{
    qDebug() << "Creating menus";
    
    // File Menu
    fileMenu = menuBar()->addMenu("&File");
    
    newAction = new QAction("&New", this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAction);
    
    openAction = new QAction("&Open", this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(openAction);
    
    saveAction = new QAction("&Save", this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    fileMenu->addAction(saveAction);
    
    fileMenu->addSeparator();
    
    exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    fileMenu->addAction(exitAction);
    
    // View Menu
    viewMenu = menuBar()->addMenu("&View");
    
    // Help Menu
    helpMenu = menuBar()->addMenu("&Help");
    QAction *aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "About SCERSE",
            "SCERSE - C Syntax Guardian v3.0\n\n"
            "Real-time error detection and syntax highlighting for C code.\n\n"
            "Built with Qt 6.9.3\n"
            "-----------------------\n"
            "By - Anirudh Sanker\n"
            "2023UCP1844\n"
            "-----------------------\n\n"
            
            "©2025");
    });
}

void MainWindow::createStatusBar()
{
    qDebug() << "Creating status bar";
    
    statusLabel = new QLabel("Ready", this);
    statusBar()->addWidget(statusLabel, 1);
    
    errorCountLabel = new QLabel("Errors: 0", this);
    statusBar()->addPermanentWidget(errorCountLabel);
    
    lineColLabel = new QLabel("Line: 1, Col: 1", this);
    statusBar()->addPermanentWidget(lineColLabel);
}

void MainWindow::setupConnections()
{
    qDebug() << "Setting up connections";
    
    // Text changes trigger debounced analysis
    // DO NOT CONNECT TO cursorPositionChanged during text change!
    connect(codeEditor, &QPlainTextEdit::textChanged,
            this, &MainWindow::onEditorTextChanged);
    
    // Timer triggers analysis pipeline
    connect(analyzeTimer, &QTimer::timeout,
            this, &MainWindow::runAnalyzerPipeline);
    
    // Error table clicks
    connect(errorTable, QOverload<int, int>::of(&QTableWidget::cellClicked),
            this, &MainWindow::onErrorTableClicked);
    
    // Cursor changes update status bar (AFTER text stabilizes)
    connect(codeEditor, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateStatusBar);
}


void MainWindow::onEditorTextChanged()
{
    qDebug() << "Editor text changed - debouncing...";
    
    isModified = true;
    if (!currentFilePath.isEmpty()) {
        setWindowTitle("SCERSE - " + currentFilePath + " *");
    }
    
    // Restart timer on text change
    analyzeTimer->stop();
    analyzeTimer->start();
    statusBar()->showMessage("Analyzing...");
}

void MainWindow::runAnalyzerPipeline()
{
    qDebug() << "=== Starting Analysis Pipeline ===";
    
    if (!codeEditor) {
        qDebug() << "ERROR: codeEditor is null";
        return;
    }
    
    QString code = codeEditor->toPlainText();
    
    if (code.isEmpty()) {
        clearAll();
        statusBar()->showMessage("Ready - No code to analyze");
        return;
    }
    
    // ===== CALL YOUR C ERROR DETECTOR =====
    CErrorDetectorEngine engine;
    AnalysisResult result = engine.analyzeCode(code.toStdString());
    
    std::vector<std::string> lexicalErrors = result.lexicalErrors;
    std::vector<std::pair<std::string, std::string>> syntaxErrors = result.syntaxErrors;
    
    // Display filtered results
    displayErrors(lexicalErrors, syntaxErrors);
    
    // REMOVE THIS - it jumps the cursor!
    // codeEditor->clearErrorHighlighting();
    // if (!syntaxErrors.empty()) {
    //     codeEditor->highlightErrorLine(1);
    // }
    
    // Instead, just highlight the error lines without moving cursor
    // Only highlight when user CLICKS on an error in the table
    
    // Update status
    int totalErrors = lexicalErrors.size() + syntaxErrors.size();
    if (totalErrors == 0) {
        statusBar()->showMessage("✓ No errors detected");
        errorCountLabel->setText("Errors: 0");
    } else {
        statusBar()->showMessage(QString("✗ Found %1 error(s)").arg(totalErrors).toStdString().c_str());
        errorCountLabel->setText(QString("Errors: %1").arg(totalErrors));
    }
    
    qDebug() << "=== Analysis Complete - Errors:" << totalErrors << "===";
}


void MainWindow::displayErrors(const std::vector<std::string> &lexErrors,
                              const std::vector<std::pair<std::string, std::string>> &syntaxErrors)
{
    qDebug() << "Displaying errors - Lex:" << lexErrors.size() << "Syntax:" << syntaxErrors.size();
    
    errorTable->setRowCount(0);
    suggestionsList->clear();
    
    int row = 0;
    QRegularExpression lineColRegex("Line (\\d+):(\\d+)");
    
    // Display lexical errors
    for (const auto &err : lexErrors) {
        errorTable->insertRow(row);
        
        // Extract line and column from error message
        QString errStr = QString::fromStdString(err);
        QRegularExpressionMatch match = lineColRegex.match(errStr);
        
        QString lineNum = "1";
        QString colNum = "1";
        
        if (match.hasMatch()) {
            lineNum = match.captured(1);
            colNum = match.captured(2);
        }
        
        // Create combined message (error + empty suggestion for lexical)
        QString fullMessage = errStr;
        
        QTableWidgetItem *lineItem = new QTableWidgetItem(lineNum);
        QTableWidgetItem *colItem = new QTableWidgetItem(colNum);
        QTableWidgetItem *msgItem = new QTableWidgetItem(fullMessage);
        
        lineItem->setForeground(QColor(255, 107, 107));  // Red
        msgItem->setForeground(QColor(255, 107, 107));
        
        errorTable->setItem(row, 0, lineItem);
        errorTable->setItem(row, 1, colItem);
        errorTable->setItem(row, 2, msgItem);
        
        row++;
    }
    
    // Display syntax errors WITH suggestions combined
    for (const auto &[err, sug] : syntaxErrors) {
        errorTable->insertRow(row);
        
        // Extract line and column from error message
        QString errStr = QString::fromStdString(err);
        QRegularExpressionMatch match = lineColRegex.match(errStr);
        
        QString lineNum = "1";
        QString colNum = "1";
        
        if (match.hasMatch()) {
            lineNum = match.captured(1);
            colNum = match.captured(2);
        }
        
        // Combine error and suggestion on ONE line for readability
        QString fullMessage = errStr;
        if (!sug.empty()) {
            fullMessage += "\n→ " + QString::fromStdString(sug);
        }
        
        QTableWidgetItem *lineItem = new QTableWidgetItem(lineNum);
        QTableWidgetItem *colItem = new QTableWidgetItem(colNum);
        QTableWidgetItem *msgItem = new QTableWidgetItem(fullMessage);
        
        lineItem->setForeground(QColor(255, 165, 0));  // Orange
        msgItem->setForeground(QColor(255, 165, 0));
        
        // Allow text to wrap
        msgItem->setTextAlignment(Qt::AlignLeft | Qt::AlignTop);
        
        errorTable->setItem(row, 0, lineItem);
        errorTable->setItem(row, 1, colItem);
        errorTable->setItem(row, 2, msgItem);
        
        // Make row taller to accommodate wrapped text
        errorTable->setRowHeight(row, 50);
        
        row++;
    }
    
    // Resize columns for better readability
    errorTable->resizeColumnToContents(0);
    errorTable->resizeColumnToContents(1);
    errorTable->horizontalHeader()->setStretchLastSection(true);
}

void MainWindow::clearAll()
{
    qDebug() << "Clearing all";
    
    errorTable->setRowCount(0);
    suggestionsList->clear();
    codeEditor->clearErrorHighlighting();
    errorCountLabel->setText("Errors: 0");
}

void MainWindow::highlightErrorLine(int lineNumber)
{
    qDebug() << "Highlighting error line:" << lineNumber;
    
    if (codeEditor) {
        codeEditor->highlightErrorLine(lineNumber);
        
        // Move cursor to the line
        QTextCursor cursor = codeEditor->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);
        codeEditor->setTextCursor(cursor);
        codeEditor->centerCursor();
    }
}

void MainWindow::onErrorTableClicked(int row, int column)
{
    Q_UNUSED(column);
    qDebug() << "Error row clicked:" << row;
    
    if (row >= 0 && row < errorTable->rowCount()) {
        bool ok = false;
        int line = errorTable->item(row, 0)->text().toInt(&ok);
        if (ok && line > 0) {
            highlightErrorLine(line);
        }
    }
}

void MainWindow::updateStatusBar()
{
    if (!codeEditor) return;
    
    QTextCursor cursor = codeEditor->textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.positionInBlock() + 1;
    lineColLabel->setText(QString("Line: %1, Col: %2").arg(line).arg(col));
}

void MainWindow::newFile()
{
    qDebug() << "New file action";
    
    if (isModified) {
        QMessageBox::StandardButton ret = QMessageBox::warning(this,
            "SCERSE",
            "The document has been modified.\nDo you want to save changes?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (ret == QMessageBox::Cancel) return;
        if (ret == QMessageBox::Save) saveFile();
    }
    
    codeEditor->clear();
    currentFilePath.clear();
    isModified = false;
    setWindowTitle("SCERSE - C Code Analyzer");
    clearAll();
    statusBar()->showMessage("New file");
}

void MainWindow::openFile()
{
    qDebug() << "Open file action";
    
    QString fileName = QFileDialog::getOpenFileName(this,
        "Open C File", "", "C Files (*.c);;Header Files (*.h);;All Files (*)");
    
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file: " + fileName);
        return;
    }
    
    QTextStream in(&file);
    codeEditor->setPlainText(in.readAll());
    file.close();
    
    currentFilePath = fileName;
    isModified = false;
    setWindowTitle("SCERSE - " + fileName);
    statusBar()->showMessage("Opened: " + fileName);
    
    qDebug() << "File opened:" << fileName;
}

void MainWindow::saveFile()
{
    qDebug() << "Save file action";
    
    if (currentFilePath.isEmpty()) {
        currentFilePath = QFileDialog::getSaveFileName(this,
            "Save C File", "", "C Files (*.c);;All Files (*)");
    }
    
    if (currentFilePath.isEmpty()) return;
    
    QFile file(currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not save file: " + currentFilePath);
        return;
    }
    
    QTextStream out(&file);
    out << codeEditor->toPlainText();
    file.close();
    
    isModified = false;
    setWindowTitle("SCERSE - " + currentFilePath);
    statusBar()->showMessage("Saved: " + currentFilePath);
    
    qDebug() << "File saved:" << currentFilePath;
}

} // namespace SCERSE
