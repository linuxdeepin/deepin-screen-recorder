// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENSHOT_SESSION_CONTROLLER_H
#define SCREENSHOT_SESSION_CONTROLLER_H

#include "../core/selectionmodel.h"
#include "../core/screenshottypes.h"

#include <QObject>
#include <QPointer>

namespace Dsr::Screenshot {

class IScreenshotBackend;
class IScreenshotOverlayHost;
class IScreenshotResultSink;
class ISnapHintSource;
class IWindowTagger;
class IAnnotationEditor;
class AnnotationCoordinator;

class ScreenshotSessionController : public QObject
{
    Q_OBJECT
public:
    enum class State {
        Idle,
        PreparingFrame,
        CreatingMasks,
        Selecting,
        Editing,
        Exporting,
        Completed,
        Cancelled,
        Failed
    };
    Q_ENUM(State)

    ScreenshotSessionController(IScreenshotBackend *backend,
                                IScreenshotOverlayHost *overlay,
                                ISnapHintSource *snapHints,
                                IWindowTagger *windowTagger,
                                IScreenshotResultSink *resultSink,
                                IAnnotationEditor *annotationEditor = nullptr,
                                QObject *parent = nullptr);
    ~ScreenshotSessionController() override;

    bool start(const Request &request);
    void cancel();
    State state() const;

signals:
    void finished();
    void failed(const QString &reason);
    void saved(const QString &path);
    void copied();

private:
    void onBackendPrepared(const FrozenFrameSet &frames);
    void onBackendFailed(const QString &reason);
    void onOverlayReady();
    void onPointerPressed(const QPoint &globalPos);
    void onPointerMoved(const QPoint &globalPos);
    void onPointerReleased(const QPoint &globalPos);
    void onSnapRegionChanged(const QRect &globalRegion);
    void onConfirmRequested();
    void onCopyRequested();
    void onCancelRequested();

    void syncOverlaySelection();
    void syncOverlayCursor(const QPoint *globalPos = nullptr);
    void startSnapHints();
    void stopSnapHints();
    void fail(const QString &reason);
    void finish(State state);
    bool renderResult(Result *result) const;

    IScreenshotBackend *m_backend = nullptr;
    IScreenshotOverlayHost *m_overlay = nullptr;
    ISnapHintSource *m_snapHints = nullptr;
    IWindowTagger *m_windowTagger = nullptr;
    IScreenshotResultSink *m_resultSink = nullptr;
    IAnnotationEditor *m_annotationEditor = nullptr;
    SelectionModel *m_selection = nullptr;
    AnnotationCoordinator *m_annotationCoordinator = nullptr;

    Request m_request;
    FrozenFrameSet m_frames;
    QRect m_snapHint;
    QPoint m_pressPos;
    State m_state = State::Idle;
    bool m_pressed = false;
    bool m_dragged = false;
    bool m_pressStartedEmpty = false;
    bool m_snapStarted = false;
    bool m_finished = false;
};

} // namespace Dsr::Screenshot

#endif // SCREENSHOT_SESSION_CONTROLLER_H
