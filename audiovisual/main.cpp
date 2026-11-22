#include "CallVarsWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("CallVars");
    QCoreApplication::setApplicationName("CallVars Audiovisual AI");
    CallVarsWindow window;
    window.show();
    return app.exec();
}
