#include "MainWindow.hpp"
#include "CodeEditor.hpp"
#include "ErrorConsole.hpp"
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

namespace CAnalyzer {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , codeEditor(nullptr)
    , errorConsole(nullptr)
    , suggestionsList(nullptr)
    , mainSplitter(nullptr)
    , analyzeTimer(nullptr)
    , statusLabel(nullptr)
    , lineColLabel(nullptr)
{
    qDebug() << "MainWindow constructor starting...";
    
    // ===== Central Widget Setup =====
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    
    // Create main vertical splitter
    mainSplitter = new QSplitter(Qt::Vertical, central);
    
    // Create components
    codeEditor = new CodeEditor(mainSplitter);
    errorConsole = new ErrorConsole(mainSplitter);
    suggestionsList = new QListWidget(mainSplitter);
    
    // Configure suggestions list
    suggestionsList->setWindowTitle("Suggestions");
    suggestionsList->setMaximumHeight(150);
    
    // Add widgets to splitter
    mainSplitter->addWidget(codeEditor);
    mainSplitter->addWidget(errorConsole);
    mainSplitter->addWidget(suggestionsList);
    
    // Set stretch factors
    mainSplitter->setStretchFactor(0, 5);  // Code editor gets most space
    mainSplitter->setStretchFactor(1, 3);  // Error console
    mainSplitter->setStretchFactor(2, 1);  // Suggestions
    
    // Layout
    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->addWidget(mainSplitter);
    layout->setContentsMargins(0, 0, 0, 0);
    central->setLayout(layout);
    
    // ===== Menus and Status Bar =====
    createMenus();
    createStatusBar();
    
    // ===== Timer Setup =====
    analyzeTimer = new QTimer(this);
    analyzeTimer->setSingleShot(true);
    analyzeTimer->setInterval(500);
    
    // ===== Connections =====
    setupConnections();
    
    // ===== Window Setup =====
    setWindowTitle("C Code Analyzer");
    resize(1400, 900);
    
    // Load sample code
    codeEditor->setPlainText(
        "#include <stdio.h>\n\n"
        "int main() {\n"
        "    int x = 10;\n"
        "    printf(\"Value: %d\\n\", x);\n"
        "    return 0;\n"
        "}\n"
    );
    
    statusBar()->showMessage("Ready");
    qDebug() << "MainWindow initialized successfully!";
}

MainWindow::~MainWindow() = default;

void MainWindow::createMenus()
{
    qDebug() << "Creating menus...";
    
    // File Menu
    fileMenu = menuBar()->addMenu("&File");
    
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
        QMessageBox::about(this, "About C Code Analyzer",
            "C Code Analyzer v1.0\n\n"
            "Syntax highlighting and error detection for C code.\n\n"
            "© 2025");
    });
}

void MainWindow::createStatusBar()
{
    qDebug() << "Creating status bar...";
    
    statusLabel = new QLabel("Ready", this);
    statusBar()->addWidget(statusLabel);
    
    lineColLabel = new QLabel("Line: 1, Col: 1", this);
    statusBar()->addPermanentWidget(lineColLabel);
}

void MainWindow::setupConnections()
{
    qDebug() << "Setting up connections...";
    
    // Text changes trigger debounced analysis
    connect(codeEditor, &QPlainTextEdit::textChanged,
            this, &MainWindow::onEditorTextChanged);
    
    // Timer triggers analysis pipeline
    connect(analyzeTimer, &QTimer::timeout,
            this, &MainWindow::runAnalyzerPipeline);
    
    // Error console clicks highlight lines
    connect(errorConsole, QOverload<int, int>::of(&ErrorConsole::cellClicked),
            this, &MainWindow::onErrorClicked);
    
    // Cursor changes update status bar
    connect(codeEditor, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateStatusBar);
}

void MainWindow::onEditorTextChanged()
{
    // Debounce: restart timer on text change
    analyzeTimer->stop();
    analyzeTimer->start();
    statusBar()->showMessage("Analyzing...");
}

void MainWindow::runAnalyzerPipeline()
{
    qDebug() << "=== Starting Analysis Pipeline ===";
    
    QString code = codeEditor->toPlainText();
    
    if (code.isEmpty()) {
        errorConsole->clearErrors();
        suggestionsList->clear();
        statusBar()->showMessage("Ready - No code to analyze");
        return;
    }
    
    // STUB: Would call your C error detector here
    // For now, just do basic validation
    std::vector<Error> errors;
    
    // Simple check: look for unclosed braces
    int openBraces = code.count('{');
    int closeBraces = code.count('}');
    
    if (openBraces != closeBraces) {
        Error err;
        err.position.line = 1;
        err.position.column = 1;
        err.message = "Unmatched braces: " + 
                     std::to_string(openBraces) + " '{' vs " + 
                     std::to_string(closeBraces) + " '}'";
        errors.push_back(err);
    }
    
    // Display errors
    errorConsole->displayErrors(errors);
    
    // Highlight first error if any
    codeEditor->clearErrorHighlighting();
    if (!errors.empty()) {
        codeEditor->highlightErrorLine(errors.position.line);
    }
    
    // Update status
    if (errors.empty()) {
        statusBar()->showMessage("✓ No errors detected");
    } else {
        statusBar()->showMessage(QString("✗ Found %1 error(s)").arg(errors.size()));
    }
    
    qDebug() << "=== Analysis Complete ===";
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

void MainWindow::onErrorClicked(int row, int column)
{
    Q_UNUSED(column);
    qDebug() << "Error row clicked:" << row;
    
    // Get line number from error console
    if (row >= 0 && row < errorConsole->rowCount()) {
        bool ok = false;
        int line = errorConsole->item(row, 0)->text().toInt(&ok);
        if (ok) {
            highlightErrorLine(line);
        }
    }
}

void MainWindow::updateStatusBar()
{
    if (!codeEditor) return;
    
    int line = codeEditor->getCurrentLine();
    int col = codeEditor->getCurrentColumn();
    lineColLabel->setText(QString("Line: %1, Col: %2").arg(line).arg(col));
}

void MainWindow::displaySuggestions(const std::vector<std::string> &suggestions)
{
    suggestionsList->clear();
    
    if (suggestions.empty()) {
        suggestionsList->addItem("No suggestions available");
        return;
    }
    
    for (const auto &suggestion : suggestions) {
        suggestionsList->addItem(QString::fromStdString(suggestion));
    }
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Open C File", "", "C Files (*.c);;Header Files (*.h);;All Files (*)");
    
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file");
        return;
    }
    
    QTextStream in(&file);
    codeEditor->setPlainText(in.readAll());
    file.close();
    
    currentFilePath = fileName;
    setWindowTitle("C Code Analyzer - " + fileName);
    statusBar()->showMessage("Opened: " + fileName);
}

void MainWindow::saveFile()
{
    if (currentFilePath.isEmpty()) {
        currentFilePath = QFileDialog::getSaveFileName(this,
            "Save C File", "", "C Files (*.c);;All Files (*)");
    }
    
    if (currentFilePath.isEmpty()) return;
    
    QFile file(currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not save file");
        return;
    }
    
    QTextStream out(&file);
    out << codeEditor->toPlainText();
    file.close();
    
    statusBar()->showMessage("Saved: " + currentFilePath);
}

} // namespace CAnalyzer
