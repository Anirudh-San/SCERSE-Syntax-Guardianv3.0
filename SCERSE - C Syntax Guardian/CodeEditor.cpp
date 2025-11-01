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
    
    setFont(QFont("Courier New", 11));
    setTabStopDistance(40);
    
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
        QColor lineColor = QColor(232, 242, 254);
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
    painter.fillRect(event->rect(), QColor(240, 240, 240));
    
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());
    
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::gray);
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
    
    // Keep current line highlight
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(230, 240, 250);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
    
    // Add error highlight
    QTextEdit::ExtraSelection errorSelection;
    errorSelection.cursor = cursor;
    errorSelection.format.setBackground(QBrush(QColor(255, 200, 200)));
    errorSelection.format.setProperty(QTextFormat::FullWidthSelection, true);
    extraSelections.append(errorSelection);
    
    setExtraSelections(extraSelections);
    
    // Move to that line
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
