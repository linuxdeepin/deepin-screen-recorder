#include <QWidget>
#include "main_window.h"
#include "single_application.h"

int main(int argc, char *argv[])
{
    SingleApplication app(argc, argv);

    if (!app.isRunning()) {
        app.setOrganizationName("deepin");
        app.setApplicationName("deepin-recorder");
        app.setApplicationVersion("1.0");
        
        MainWindow window;

        QObject::connect(&app, SIGNAL(secondInstanceStart()), &window, SLOT(stopRecord()));
        
        window.setWindowTitle("Deepin recorder");
        window.setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        window.setAttribute(Qt::WA_TranslucentBackground, true);
        window.showFullScreen();
        window.show();

        return app.exec();
    }

    return 0;
}
