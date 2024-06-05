#include "recorderwindow.h"
#include "config.h"

RecorderWindow::RecorderWindow(Status status, QScreen *screen, QQmlEngine *engine, QWindow *parent)
    : BaseWindow(engine, parent)
    , m_screenToFollow(screen)
{
    m_context->setContextObject(this); // Must be before QML is initialized

    setFlags({
        Qt::Window, // the default window flag
        Qt::FramelessWindowHint,
        Qt::NoDropShadowWindowHint,
        Qt::MaximizeUsingFullscreenGeometryHint // also use the areas where system UIs are
    });

    setWindowStates(Qt::WindowFullScreen);

    this->setColor(Qt::transparent);
    // set up QML
    setStatus(status); // sets source and other stuff based on mode.
}

RecorderWindow::~RecorderWindow()
{

}

void RecorderWindow::setStatus(RecorderWindow::Status status)
{
    if (status == shot) {
        setSource(u"%1/gui/ImageOverlay.qml"_qs.arg(SCREENRECORDERCORE_QML_PATH));
    }
}
