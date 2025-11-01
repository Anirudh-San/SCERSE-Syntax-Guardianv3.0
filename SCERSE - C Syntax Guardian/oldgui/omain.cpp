#include <QApplication>
#include "MainWindow.hpp"

using namespace CAnalyzer;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
