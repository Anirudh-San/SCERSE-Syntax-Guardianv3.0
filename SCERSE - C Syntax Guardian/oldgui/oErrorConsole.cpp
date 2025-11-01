#include "ErrorConsole.hpp"
#include <QDebug>

namespace CAnalyzer {

ErrorConsole::ErrorConsole(QWidget *parent)
    : QTableWidget(parent)
{
    qDebug() << "ErrorConsole constructor";
    
    setColumnCount(3);
    setHorizontalHeaderLabels({"Line", "Column", "Message"});
    horizontalHeader()->setStretchLastSection(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    connect(this, QOverload<int, int>::of(&QTableWidget::cellClicked),
            this, &ErrorConsole::onCellClicked);
}

void ErrorConsole::displayErrors(const std::vector<Error> &errors)
{
    qDebug() << "Displaying" << errors.size() << "errors";
    
    setRowCount(static_cast<int>(errors.size()));
    
    for (size_t i = 0; i < errors.size(); ++i) {
        setItem(static_cast<int>(i), 0, new QTableWidgetItem(
            QString::number(errors[i].position.line)));
        setItem(static_cast<int>(i), 1, new QTableWidgetItem(
            QString::number(errors[i].position.column)));
        setItem(static_cast<int>(i), 2, new QTableWidgetItem(
            QString::fromStdString(errors[i].message)));
    }
}

void ErrorConsole::clearErrors()
{
    setRowCount(0);
}

void ErrorConsole::onCellClicked(int row, int column)
{
    Q_UNUSED(column);
    
    if (row < 0 || row >= rowCount()) return;
    
    bool ok = false;
    int line = item(row, 0)->text().toInt(&ok);
    if (ok) {
        qDebug() << "Error clicked at line:" << line;
    }
}

} // namespace CAnalyzer
