// ============================================================================
// FILE 1: MainWindow.hpp
// ============================================================================

#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QListWidget>
#include <QLabel>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QTableWidget>


namespace SCERSE {

class CodeEditor;
class SyntaxHighlighter;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onEditorTextChanged();
    void runAnalyzerPipeline();
    void highlightErrorLine(int lineNumber);
    void onErrorTableClicked(int row, int column);
    void updateStatusBar();
    void openFile();
    void saveFile();
    void newFile();

private:
    // UI Components
    CodeEditor *codeEditor;
    QTableWidget *errorTable;
    QListWidget *suggestionsList;
    QSplitter *mainSplitter;
    
    // Timer for debounced analysis
    QTimer *analyzeTimer;
    
    // Status bar
    QLabel *statusLabel;
    QLabel *lineColLabel;
    QLabel *errorCountLabel;
    
    // Menus & Actions
    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;
    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *exitAction;
    
    // File path
    QString currentFilePath;
    bool isModified;
    
    // Helper methods
    void createMenus();
    void createStatusBar();
    void setupConnections();
    void setupErrorTable();
    void displayErrors(const std::vector<std::string> &lexErrors,
                      const std::vector<std::pair<std::string, std::string>> &syntaxErrors);
    void clearAll();
};

} // namespace SCERSE
