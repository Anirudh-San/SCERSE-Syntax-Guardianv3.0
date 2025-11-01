#pragma once

#include <QSyntaxHighlighter>
#include <QVector>
#include <QTextCharFormat>
#include <QRegularExpression>

namespace CAnalyzer {

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit SyntaxHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    
    QVector<HighlightingRule> highlightingRules;
    QTextCharFormat keywordFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat operatorFormat;
};

} // namespace CAnalyzer
