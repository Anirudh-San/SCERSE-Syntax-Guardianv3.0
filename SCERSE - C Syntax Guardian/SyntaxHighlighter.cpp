#include "SyntaxHighlighter.hpp"
#include <QDebug>

namespace SCERSE {

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    qDebug() << "SyntaxHighlighter constructor";
    
    HighlightingRule rule;
    
    // Keywords (bold blue)
    keywordFormat.setForeground(QColor(0, 0, 255));
    keywordFormat.setFontWeight(QFont::Bold);
    
    QStringList keywords;
    keywords << "\\bint\\b" << "\\bfloat\\b" << "\\bchar\\b" << "\\bvoid\\b"
             << "\\bdouble\\b" << "\\bif\\b" << "\\belse\\b" << "\\bwhile\\b"
             << "\\bfor\\b" << "\\breturn\\b" << "\\bstruct\\b" << "\\btypedef\\b"
             << "\\bconst\\b" << "\\bstatic\\b" << "\\bextern\\b" << "\\bsizeof\\b"
             << "\\bbreak\\b" << "\\bcontinue\\b" << "\\bswitch\\b" << "\\bcase\\b"
             << "\\bdefault\\b" << "\\bunion\\b" << "\\benum\\b" << "\\bauto\\b";
    
    for (const QString &keyword : keywords) {
        rule.pattern = QRegularExpression(keyword);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
    
    // Preprocessor (magenta)
    preprocessorFormat.setForeground(QColor(128, 0, 255));
    rule.pattern = QRegularExpression("#\\b\\w+");
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);
    
    // Numbers (dark magenta)
    numberFormat.setForeground(QColor(139, 0, 139));
    rule.pattern = QRegularExpression("\\b[0-9]+\\.?[0-9]*\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);
    
    // Strings (dark green)
    stringFormat.setForeground(QColor(0, 128, 0));
    rule.pattern = QRegularExpression("\".*?\"|'.*?'");
    rule.format = stringFormat;
    highlightingRules.append(rule);
    
    // Operators (dark red)
    operatorFormat.setForeground(QColor(139, 0, 0));
    QStringList operators;
    operators << "\\+" << "-" << "\\*" << "/" << "%" << "="
              << "==" << "!=" << "<" << "<=" << ">" << ">="
              << "&&" << "\\|\\|" << "!" << ";" << "," << "\\." 
              << "\\(|\\)|\\{|\\}|\\[|\\]" << "&" << "\\|" << "\\^";
    
    for (const QString &op : operators) {
        rule.pattern = QRegularExpression(op);
        rule.format = operatorFormat;
        highlightingRules.append(rule);
    }
    
    qDebug() << "SyntaxHighlighter initialized with" << highlightingRules.size() << "rules";
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    // Apply all highlighting rules
    for (const HighlightingRule &rule : std::as_const(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    
    // Single-line comments
    commentFormat.setForeground(QColor(128, 128, 128));
    commentFormat.setFontItalic(true);
    QRegularExpression singleLineCommentExpression("//[^\\n]*");
    QRegularExpressionMatchIterator matchIterator = singleLineCommentExpression.globalMatch(text);
    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        setFormat(match.capturedStart(), match.capturedLength(), commentFormat);
    }
    
    // Multi-line comment support
    QRegularExpression startExpression("/\\*");
    QRegularExpression endExpression("\\*/");
    
    int startIndex = 0;
    if (previousBlockState() != 1) {
        startIndex = text.indexOf(startExpression);
    }
    
    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = endExpression.match(text, startIndex);
        int endIndex = endMatch.capturedStart();
        int commentLength;
        
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        
        setFormat(startIndex, commentLength, commentFormat);
        startIndex = text.indexOf(startExpression, startIndex + commentLength);
    }
}

} // namespace SCERSE
