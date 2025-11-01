#include "SyntaxHighlighter.hpp"

namespace CAnalyzer {

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;
    
    // Keywords (bold blue)
    keywordFormat.setForeground(QColor(0, 0, 255));
    keywordFormat.setFontWeight(QFont::Bold);
    
    QStringList keywords;
    keywords << "\\bint\\b" << "\\bfloat\\b" << "\\bchar\\b" << "\\bvoid\\b"
             << "\\bdouble\\b" << "\\bif\\b" << "\\belse\\b" << "\\bwhile\\b"
             << "\\bfor\\b" << "\\breturn\\b" << "\\bstruct\\b" << "\\btypedef\\b"
             << "\\bconst\\b" << "\\bstatic\\b" << "\\bextern\\b" << "\\bsizeof\\b";
    
    for (const QString &keyword : keywords) {
        rule.pattern = QRegularExpression(keyword);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
    
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
              << "&&" << "\\|\\|" << "!" << ";" << "," << "\\." << "\\(|\\)|\\{|\\}|\\[|\\]";
    
    for (const QString &op : operators) {
        rule.pattern = QRegularExpression(op);
        rule.format = operatorFormat;
        highlightingRules.append(rule);
    }
    
    // Comments (gray, italic)
    commentFormat.setForeground(QColor(128, 128, 128));
    commentFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("//[^\\n]*");
    rule.format = commentFormat;
    highlightingRules.append(rule);
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

} // namespace CAnalyzer
