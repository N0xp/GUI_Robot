#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Set application metadata
    QApplication::setApplicationName("Robot Path Planner");
    QApplication::setOrganizationName("FRC Team");
    QApplication::setApplicationVersion("1.0");

    // Create and show main window
    MainWindow window;
    window.show();

    return app.exec();
}
