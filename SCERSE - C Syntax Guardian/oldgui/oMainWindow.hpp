#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QListWidget>
#include <QLabel>
#include <QTimer>
#include <QAction>
#include <QMenu>

class CodeEditor;
class ErrorConsole;
class SyntaxHighlighter;

namespace CAnalyzer {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onEditorTextChanged();
    void runAnalyzerPipeline();
    void highlightErrorLine(int lineNumber);
    void onErrorClicked(int row, int column);
    void updateStatusBar();
    void openFile();
    void saveFile();

private:
    // UI Components
    CodeEditor *codeEditor;
    ErrorConsole *errorConsole;
    QListWidget *suggestionsList;
    QSplitter *mainSplitter;
    
    // Timer for debounced analysis
    QTimer *analyzeTimer;
    
    // Status bar
    QLabel *statusLabel;
    QLabel *lineColLabel;
    
    // Menus & Actions
    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;
    QAction *openAction;
    QAction *saveAction;
    QAction *exitAction;
    
    // File path
    QString currentFilePath;
    
    // Helper methods
    void createMenus();
    void createStatusBar();
    void setupConnections();
    void displaySuggestions(const std::vector<std::string> &suggestions);
};

} // namespace CAnalyzer
