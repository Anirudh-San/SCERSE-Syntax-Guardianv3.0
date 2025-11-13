#include "CodeEditor.hpp"
#include "SyntaxHighlighter.hpp"

#include <QPainter>
#include <QTextBlock>
#include <QResizeEvent>
#include <QDebug>
#include <QFont>

namespace SCERSE {

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , lineNumberArea(nullptr)
    , syntaxHighlighter(nullptr)
{
    qDebug() << "CodeEditor constructor";
    
    lineNumberArea = new LineNumberArea(this);
    syntaxHighlighter = new SyntaxHighlighter(this->document());
    
    connect(this, &CodeEditor::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);
    
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    
    // Font settings
    setFont(QFont("Courier New", 11));
    setTabStopDistance(40);
    
    // ===== DARK MODE FIX: Set proper colors =====
    
    // Set stylesheet for dark theme compatibility
    setStyleSheet(
        "QPlainTextEdit {"
        "   background-color: #1e1e1e;"    // Dark background
        "   color: #d4d4d4;"                 // Light gray text
        "   selection-background-color: #264f78;"  // Blue selection
        "   selection-color: #ffffff;"       // White selected text
        "}"
    );
    
    /* 
    // Alternative: Use QPalette for more control
    QPalette p = palette();
    p.setColor(QPalette::Base, QColor(30, 30, 30));           // Background: dark gray
    p.setColor(QPalette::Text, QColor(212, 212, 212));        // Text: light gray
    p.setColor(QPalette::Highlight, QColor(38, 79, 120));     // Selection background
    p.setColor(QPalette::HighlightedText, QColor(255, 255, 255)); // Selected text
    setPalette(p);
    */

    qDebug() << "CodeEditor initialized";
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    
    return 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void CodeEditor::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::highlightCurrentLine() {
    // Save cursor state
    auto oldCursor = textCursor();

    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        
        // DARK MODE: Use darker highlight for current line
        QColor lineColor = QColor(44, 44, 44);  // Dark gray (was 232, 242, 254)
        
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
    
    // Restore cursor state
    setTextCursor(oldCursor);
}



void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy) {
        lineNumberArea->scroll(0, dy);
    } else {
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    }
    
    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    
    // DARK MODE: Dark background for line numbers
    painter.fillRect(event->rect(), QColor(37, 37, 37));  // Dark gray (was 240, 240, 240)
    
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());
    
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            
            // DARK MODE: Light gray text for line numbers
            painter.setPen(QColor(128, 128, 128));  // Gray (was Qt::gray)
            
            painter.drawText(0, top, lineNumberArea->width() - 4,
                           fontMetrics().height(),
                           Qt::AlignRight, number);
        }
        
        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::highlightErrorLine(int lineNumber)
{
    qDebug() << "Highlighting error at line:" << lineNumber;
    
    QTextCursor cursor(document()->findBlockByNumber(lineNumber - 1));
    
    QList<QTextEdit::ExtraSelection> extraSelections;
    
    // Keep current line highlight (DARK MODE color)
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(44, 44, 44);  // Dark gray
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
    
    // Add error highlight (DARK MODE: lighter red)
    QTextEdit::ExtraSelection errorSelection;
    errorSelection.cursor = cursor;
    errorSelection.format.setBackground(QBrush(QColor(100, 30, 30))); // Dark red (was 255, 200, 200)
    errorSelection.format.setProperty(QTextFormat::FullWidthSelection, true);
    extraSelections.append(errorSelection);
    
    setExtraSelections(extraSelections);
    
    // Move to that line
    setTextCursor(cursor);
    centerCursor();
}

void CodeEditor::highlightErrorColumn(int lineNumber, int startColumn, int endColumn) {
    QTextDocument *doc = document();
    QTextBlock block = doc->findBlockByNumber(lineNumber - 1);
    
    if (!block.isValid()) return;
    
    QTextCursor cursor(block);
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, startColumn - 1);
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, endColumn - startColumn + 1);
    
    QList<QTextEdit::ExtraSelection> extraSelections;
    
    // Error highlight for specific column range
    QTextEdit::ExtraSelection errorSelection;
    errorSelection.cursor = cursor;
    errorSelection.format.setBackground(QBrush(QColor(200, 0, 0)));  // Red background
    errorSelection.format.setForeground(QBrush(QColor(255, 255, 255)));  // White text
    extraSelections.append(errorSelection);
    
    setExtraSelections(extraSelections);
    
    // Move to error position
    setTextCursor(cursor);
    centerCursor();
}

void CodeEditor::clearErrorHighlighting()
{
    highlightCurrentLine();
}

int CodeEditor::getCurrentLine() const
{
    QTextCursor cursor = textCursor();
    return cursor.blockNumber() + 1;
}

int CodeEditor::getCurrentColumn() const
{
    QTextCursor cursor = textCursor();
    return cursor.positionInBlock() + 1;
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                     lineNumberAreaWidth(), cr.height()));
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    codeEditor->lineNumberAreaPaintEvent(event);
}

} // namespace SCERSE
