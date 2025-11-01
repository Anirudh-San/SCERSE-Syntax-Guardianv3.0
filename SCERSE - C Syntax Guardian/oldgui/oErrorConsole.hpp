#pragma once

#include <QTableWidget>
#include <vector>

namespace CAnalyzer {

struct Position {
    int line = 0;
    int column = 0;
};

struct Error {
    Position position;
    std::string message;
};

class ErrorConsole : public QTableWidget {
    Q_OBJECT

public:
    explicit ErrorConsole(QWidget *parent = nullptr);
    
    void displayErrors(const std::vector<Error> &errors);
    void clearErrors();

private slots:
    void onCellClicked(int row, int column);

private:
};

} // namespace CAnalyzer
