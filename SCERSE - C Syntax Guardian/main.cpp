// ============================================================================
// FILE 7: main.cpp
// ============================================================================

#include <QApplication>
#include <QDebug>
#include "MainWindow.hpp"

using namespace SCERSE;

int main(int argc, char *argv[])
{
    qDebug() << "=== Starting SCERSE Application ===";
    
    QApplication app(argc, argv);
    
    MainWindow window;
    window.show();
    
    qDebug() << "=== Entering Event Loop ===";
    return app.exec();
}
