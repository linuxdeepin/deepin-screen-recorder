// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenshot/core/selectionmodel.h"
#include "screenshot/core/selectionoverlaygeometry.h"
#include "screenshot/export/screenshotexportservice.h"
#include "screenshot/ports/iannotationeditor.h"
#include "screenshot/ports/irecordingsession.h"
#include "screenshot/ports/iscrollcapture.h"
#include "screenshot/ports/iscrollmerger.h"
#include "screenshot/core/inputregiongeometry.h"
#include "screenshot/core/scrolltipposition.h"
#include "screenshot/ports/iscreenshotbackend.h"
#include "screenshot/ports/iscreenshotoverlayhost.h"
#include "screenshot/ports/iscreenshotresultsink.h"
#include "screenshot/merge/scrollmergecoordinator.h"
#include "screenshot/ports/isnaphintsource.h"
#include "screenshot/ports/iwindowtagger.h"
#include "screenshot/platform/treeland/recordingfiletransaction.h"
#include "screenshot/platform/treeland/treelandscrollcompositor.h"
#include "ext-image-capture/framepumpworker.h"
#include "screenshot/session/annotationcoordinator.h"
#include "screenshot/session/platformdetection.h"
#include "screenshot/session/screenshotsessioncontroller.h"
#include "utils/imageeffects.h"
#include "utils/shapesutils.h"
#include "utils/pixmergethread.h"
#include "utils.h"

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QSignalSpy>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QTemporaryDir>
#include <QtTest>

using namespace Dsr::Screenshot;

namespace {

OutputFrame makeFrame(const QRect &geometry, const QColor &color, qreal scale = 1.0)
{
    OutputFrame frame;
    frame.output.outputId = QStringLiteral("output-%1").arg(geometry.x());
    frame.output.logicalGeometry = geometry;
    frame.output.devicePixelRatio = scale;
    frame.pixelScale = scale;
    frame.image = QImage(qRound(geometry.width() * scale),
                         qRound(geometry.height() * scale),
                         QImage::Format_ARGB32_Premultiplied);
    frame.image.fill(color);
    return frame;
}


QImage makeScrollSource(int width, int height)
{
    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    QRandomGenerator random(0x5c2011);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int value = static_cast<int>(random.generate() & 0xff);
            image.setPixelColor(x, y,
                                QColor(value,
                                       (value + x * 7 + y * 3) & 0xff,
                                       (value * 3 + x * 5 + y * 11) & 0xff));
        }
    }
    return image;
}

QPixmap scrollViewport(const QImage &source, int top, int height)
{
    return QPixmap::fromImage(source.copy(0, top, source.width(), height));
}

bool waitForMergeQueue(PixMergeThread &merge, int expected, int timeoutMs = 5000)
{
    QElapsedTimer timer;
    timer.start();
    while (merge.pendingImageCount() != expected && timer.elapsed() < timeoutMs)
        QTest::qWait(5);
    return merge.pendingImageCount() == expected;
}

class FakeAnnotationEditor : public IAnnotationEditor
{
    Q_OBJECT
public:
    QRectF bounds;
    QColor renderColor = Qt::transparent;
    mutable bool renderCalled = false;
    AnnotationHit currentHit;
    bool beginResult = false;
    bool interacting = false;
    mutable int hitTestCount = 0;
    int beginCount = 0;
    int updateCount = 0;
    int endCount = 0;
    int cancelCount = 0;
    int clearCount = 0;
    bool selectedContent = false;
    QList<QPoint> updatePositions;
    AnnotationTool currentTool = AnnotationTool::Pointer;
    QRect interactionBounds;

    QRectF contentBoundsGlobal() const override
    {
        return bounds;
    }

    bool isInteracting() const override
    {
        return interacting;
    }

    void setTool(AnnotationTool tool) override
    {
        currentTool = tool;
    }

    AnnotationTool tool() const override
    {
        return currentTool;
    }

    void setInteractionBounds(const QRect &globalBounds) override
    {
        interactionBounds = globalBounds;
    }

    bool hasSelectedContent() const override
    {
        return selectedContent;
    }

    AnnotationHit hitTest(const QPoint &) const override
    {
        ++hitTestCount;
        return currentHit;
    }

    bool beginInteraction(const QPoint &) override
    {
        ++beginCount;
        interacting = beginResult;
        return beginResult;
    }

    void updateInteraction(const QPoint &globalPos) override
    {
        ++updateCount;
        updatePositions.append(globalPos);
    }

    void endInteraction() override
    {
        ++endCount;
        interacting = false;
    }

    void cancelInteraction() override
    {
        ++cancelCount;
        interacting = false;
    }

    void clearContent() override
    {
        ++clearCount;
        bounds = QRectF();
        emit contentChanged();
        emit contentBoundsChanged(bounds);
    }

    void render(QPainter &painter, const QRect &, qreal) const override
    {
        renderCalled = true;
        if (renderColor.alpha() > 0)
            painter.fillRect(QRectF(0, 0, 10000, 10000), renderColor);
    }

    void setBounds(const QRectF &newBounds)
    {
        bounds = newBounds;
        emit contentBoundsChanged(bounds);
    }
};

class FakeBackend : public IScreenshotBackend
{
    Q_OBJECT
public:
    FrozenFrameSet frames;
    int prepareCount = 0;

    BackendCapabilities capabilities() const override
    {
        return {};
    }

    QList<OutputDescriptor> outputs() const override
    {
        QList<OutputDescriptor> result;
        for (const OutputFrame &frame : frames.outputs)
            result.append(frame.output);
        return result;
    }

    void prepare(const Request &) override
    {
        ++prepareCount;
        emit prepared(frames);
    }

    void cancel() override
    {
    }
};

class FakeRecordingSession : public IRecordingSession
{
    Q_OBJECT
public:
    int startCount = 0;
    int stopCount = 0;
    int cancelCount = 0;
    RecordingRequest lastRequest;
    State currentState = State::Idle;
    bool startResult = true;

    bool start(const RecordingRequest &request) override
    {
        ++startCount;
        lastRequest = request;
        if (!startResult)
            return false;
        currentState = State::Recording;
        emit started();
        return true;
    }

    void stop() override
    {
        ++stopCount;
    }

    void cancel() override
    {
        ++cancelCount;
    }

    State state() const override
    {
        return currentState;
    }

    QString outputPath() const override
    {
        return lastRequest.outputPath;
    }
};

class FakeOverlay : public IScreenshotOverlayHost
{
    Q_OBJECT
public:
    QList<OutputDescriptor> createdOutputs;
    QRect selection;
    QRect snapHint;
    bool finalized = false;
    bool shown = false;
    bool closed = false;
    bool clearSelectionOnCreate = false;
    bool readyOnShow = true;
    bool prepareResult = true;
    int prepareCount = 0;
    QStringList lifecycle;
    IAnnotationEditor *pendingEditor = nullptr;
    IAnnotationEditor *providedEditor = nullptr;
    CursorIntent cursorIntent = CursorIntent::Default;
    QString magnifierHex;
    bool hintsVisible = false;
    bool hintsHasGlobalCursor = false;
    QPoint hintsCursor;
    bool scrollActive = false;
    int scrollBeginCount = 0;
    int scrollFinishCount = 0;
    QList<bool> scrollFinishRestoreArgs;
    bool scrollBeginResult = true;
    QRect scrollSelection;
    int scrollPreviewCount = 0;
    QImage lastScrollPreview;
    int scrollFrameCaptureBeginCount = 0;
    int scrollFrameCaptureEndCount = 0;
    int scrollTipShowCount = 0;
    int scrollTipHideCount = 0;
    ScrollTipKind lastScrollTip = ScrollTipKind::None;
    Mode initialMode = Mode::Region;
    bool hasInitialMode = false;
    bool recordModeActive = false;
    bool countdownResult = true;
    bool emitCountdownImmediately = false;
    int countdownStartCount = 0;
    int countdownStopCount = 0;
    bool beginRecordingResult = true;
    int beginRecordingCount = 0;
    bool recordingActive = false;

    bool create(const QList<OutputDescriptor> &outputs) override
    {
        if (clearSelectionOnCreate)
            selection = QRect();
        providedEditor = pendingEditor;
        createdOutputs = outputs;
        return !outputs.isEmpty();
    }

    IAnnotationEditor *annotationEditor() const override
    {
        return providedEditor;
    }

    QList<QWindow *> maskWindows() const override
    {
        return {};
    }

    void setFrozenFrames(const FrozenFrameSet &) override
    {
    }

    bool prepare() override
    {
        ++prepareCount;
        lifecycle.append(QStringLiteral("prepare"));
        return prepareResult;
    }

    void show() override
    {
        shown = true;
        lifecycle.append(QStringLiteral("show"));
        if (readyOnShow)
            emit inputReady();
    }

    void setSelection(const QRect &globalSelection) override
    {
        selection = globalSelection;
    }

    void setSnapHint(const QRect &globalRegion) override
    {
        snapHint = globalRegion;
    }

    void setInitialMode(Mode mode) override
    {
        initialMode = mode;
        hasInitialMode = true;
    }

    void setRecordModeActive(bool active) override
    {
        recordModeActive = active;
    }

    void setSelectionFinalized(bool value) override
    {
        finalized = value;
    }

    void setCursorIntent(CursorIntent intent) override
    {
        cursorIntent = intent;
    }

    QString magnifierColorHex() const override
    {
        return magnifierHex;
    }

    void setSelectionHints(bool visible,
                           bool,
                           const QPoint &globalCursor,
                           bool hasGlobalCursor) override
    {
        hintsVisible = visible;
        hintsCursor = globalCursor;
        hintsHasGlobalCursor = hasGlobalCursor;
    }

    bool beginScroll(const QRect &globalSelection) override
    {
        ++scrollBeginCount;
        scrollSelection = globalSelection;
        scrollActive = scrollBeginResult;
        return scrollBeginResult;
    }

    void finishScroll(bool restoreSelectionUi = true) override
    {
        ++scrollFinishCount;
        scrollFinishRestoreArgs.append(restoreSelectionUi);
        scrollActive = false;
    }

    bool startRecordingCountdown(const QRect &) override
    {
        ++countdownStartCount;
        if (emitCountdownImmediately)
            emit recordingCountdownFinished();
        return countdownResult;
    }

    void stopRecordingCountdown() override
    {
        ++countdownStopCount;
    }

    bool beginRecording(const QRect &) override
    {
        ++beginRecordingCount;
        recordingActive = beginRecordingResult;
        return beginRecordingResult;
    }

    void finishRecording() override
    {
        recordingActive = false;
    }

    bool isRecordingActive() const override
    {
        return recordingActive;
    }

    bool isScrollActive() const override
    {
        return scrollActive;
    }

    void setScrollPreview(const QImage &image) override
    {
        ++scrollPreviewCount;
        lastScrollPreview = image;
    }

    void beginScrollFrameCapture() override
    {
        ++scrollFrameCaptureBeginCount;
    }

    void endScrollFrameCapture() override
    {
        ++scrollFrameCaptureEndCount;
    }

    void showScrollTip(ScrollTipKind kind) override
    {
        ++scrollTipShowCount;
        lastScrollTip = kind;
    }

    void hideScrollTip() override
    {
        ++scrollTipHideCount;
    }

    void close() override
    {
        closed = true;
    }

    void scrollStart()
    {
        emit scrollStartRequested();
    }

    void scrollStop()
    {
        emit scrollStopRequested();
    }

    void press(const QPoint &position)
    {
        emit pointerPressed(position);
    }

    void move(const QPoint &position)
    {
        emit pointerMoved(position);
    }

    void release(const QPoint &position)
    {
        emit pointerReleased(position);
    }

    void confirm()
    {
        emit confirmRequested();
    }

    void copy()
    {
        emit copyRequested();
    }

    void pin()
    {
        emit pinRequested();
    }

    void ocr()
    {
        emit ocrRequested();
    }

    void tableRecognize()
    {
        emit tableRecognizeRequested();
    }

    void aiAssistant(int function)
    {
        emit aiAssistantRequested(function);
    }

    void cancel()
    {
        emit cancelRequested();
    }

    void failInput(const QString &reason)
    {
        emit inputFailed(reason);
    }

    void requestSelectionMove(const QPoint &delta)
    {
        emit selectionMoveRequested(delta);
    }

    void requestSelectionResize(Qt::Key key, Qt::KeyboardModifiers modifiers)
    {
        emit selectionResizeRequested(key, modifiers);
    }

    void requestSelectionSize(int width, int height, qreal pixelScale)
    {
        emit selectionSizeChangeRequested(width, height, pixelScale);
    }

    void requestSelectionRatio(int width, int height)
    {
        emit selectionRatioChangeRequested(width, height);
    }

    void requestRoundedCornerRadius(int radius)
    {
        emit roundedCornerRadiusChanged(radius);
    }

    void requestRecordMode(bool enabled)
    {
        emit recordModeChanged(enabled);
    }
};

class FakeSnapHints : public ISnapHintSource
{
    Q_OBJECT
public:
    bool start(QString *) override
    {
        return true;
    }

    void stop() override
    {
    }

    QRect currentRegion() const override
    {
        return {};
    }

    void setRegion(const QRect &region)
    {
        emit regionChanged(region);
    }
};

class FakeWindowTagger : public IWindowTagger
{
    Q_OBJECT
public:
    bool tagMaskWindow(QWindow *, QString *) override
    {
        return true;
    }
};

class FakeResultSink : public IScreenshotResultSink
{
    Q_OBJECT
public:
    int saveCount = 0;
    int copyCount = 0;
    int textCopyCount = 0;
    int pinCount = 0;
    int ocrCount = 0;
    int tableRecognizeCount = 0;
    int aiAssistantCount = 0;
    int aiAssistantFunction = -1;
    QString requestedPath;
    QString copiedText;
    Result lastResult;
    bool saveResult = true;
    bool saveCancelled = false;
    bool textCopyResult = true;
    bool ocrResult = true;
    bool tableRecognizeResult = true;
    bool aiAssistantResult = true;
    bool notificationsEnabled = true;
    int notificationSettingCount = 0;
    QString saveError;
    QString textCopyError;
    QString ocrError;
    QString tableRecognizeError;
    QString aiAssistantError;

    bool save(const Result &result,
              const QString &path,
              QString *savedPath,
              QString *error,
              bool *cancelled) override
    {
        ++saveCount;
        requestedPath = path;
        lastResult = result;
        if (cancelled)
            *cancelled = saveCancelled;
        if (error)
            *error = saveError;
        if (savedPath)
            *savedPath = path;
        return saveResult;
    }

    bool copy(const Result &result, QString *) override
    {
        ++copyCount;
        lastResult = result;
        return true;
    }

    bool copyText(const QString &text, QString *error) override
    {
        ++textCopyCount;
        copiedText = text;
        if (error)
            *error = textCopyError;
        return textCopyResult;
    }

    bool pin(const Result &result, QString *) override
    {
        ++pinCount;
        lastResult = result;
        return true;
    }

    bool ocr(const Result &result, QString *error) override
    {
        ++ocrCount;
        lastResult = result;
        if (error)
            *error = ocrError;
        return ocrResult;
    }

    bool tableRecognize(const Result &result, QString *error) override
    {
        ++tableRecognizeCount;
        lastResult = result;
        if (error)
            *error = tableRecognizeError;
        return tableRecognizeResult;
    }

    bool aiAssistant(const Result &result, int function, QString *error) override
    {
        ++aiAssistantCount;
        aiAssistantFunction = function;
        lastResult = result;
        if (error)
            *error = aiAssistantError;
        return aiAssistantResult;
    }

    void setNotificationsEnabled(bool enabled) override
    {
        notificationsEnabled = enabled;
        ++notificationSettingCount;
    }
};

class FakeScrollCapture : public IScrollCapture
{
    Q_OBJECT
public:
    int startCount = 0;
    int stopCount = 0;
    int pauseCount = 0;
    int requestCount = 0;
    int resumeCount = 0;
    bool startResult = true;
    bool replayFrameOnRequest = false;
    bool failOnRequest = false;
    QRect selection;
    QList<OutputDescriptor> outputs;
    ScrollFrame lastFrame;

    bool start(const QList<OutputDescriptor> &newOutputs,
               const QRect &globalSelection,
               QString *) override
    {
        ++startCount;
        outputs = newOutputs;
        selection = globalSelection;
        return startResult;
    }

    void requestFrame() override
    {
        ++requestCount;
        if (failOnRequest) {
            emit failed(QStringLiteral("等待 Wayland 事件超时"));
            return;
        }
        if (replayFrameOnRequest)
            emit frameReady(lastFrame);
    }

    void pause() override
    {
        ++pauseCount;
    }

    void resume() override
    {
        ++resumeCount;
    }

    void stop() override
    {
        ++stopCount;
    }

    void emitFrame(const ScrollFrame &frame)
    {
        lastFrame = frame;
        emit frameReady(frame);
    }

    void fail(const QString &reason)
    {
        emit failed(reason);
    }

    void failFrame(const QString &reason)
    {
        emit frameFailed(reason);
    }

    void emitCaptureAboutToStart()
    {
        emit captureAboutToStart();
    }

    void emitCaptureFinished()
    {
        emit captureFinished();
    }
};

class FakeScrollMerger : public IScrollImageMerger
{
    Q_OBJECT
public:
    int startCount = 0;
    int addCount = 0;
    int finishCount = 0;
    int cancelCount = 0;
    QRect selection;
    QList<ScrollFrame> frames;
    QImage result;

    void start(const QRect &globalSelection) override
    {
        ++startCount;
        selection = globalSelection;
    }

    bool addFrame(const ScrollFrame &frame) override
    {
        ++addCount;
        frames.append(frame);
        return true;
    }

    QImage finish() override
    {
        ++finishCount;
        return result;
    }

    void cancel() override
    {
        ++cancelCount;
    }

    void emitReachedBottom()
    {
        emit reachedBottom();
    }

    void emitPreview(const QImage &image)
    {
        emit previewUpdated(image);
    }

    void fail(const QString &reason)
    {
        emit failed(reason);
    }

    void emitQuickScroll()
    {
        emit quickScrollDetected();
    }

    void emitInvalidArea()
    {
        emit invalidAreaDetected();
    }
};

class ScopedEnvVar
{
public:
    ScopedEnvVar(const char *name, const QByteArray &value)
        : m_name(name)
        , m_wasSet(qEnvironmentVariableIsSet(name))
        , m_oldValue(qgetenv(name))
    {
        qputenv(name, value);
    }

    ~ScopedEnvVar()
    {
        if (m_wasSet)
            qputenv(m_name, m_oldValue);
        else
            qunsetenv(m_name);
    }

private:
    QByteArray m_name;
    bool m_wasSet = false;
    QByteArray m_oldValue;
};

} // namespace

class ScreenshotSessionTest : public QObject
{
    Q_OBJECT
private slots:
    void treeLandDetectionIgnoresPlainWaylandSessions()
    {
        ScopedEnvVar demoMode("DSR_TREELAND_DEMO", QByteArray());
        ScopedEnvVar compositor("DDE_CURRENT_COMPOSITOR", QByteArray("KWin"));

        QVERIFY(!isTreeLandEnvironment());

        qputenv("DDE_CURRENT_COMPOSITOR", "wayland");
        QVERIFY(!isTreeLandEnvironment());

        qputenv("DDE_CURRENT_COMPOSITOR", "TreeLand");
        QVERIFY(isTreeLandEnvironment());

        qputenv("DDE_CURRENT_COMPOSITOR", "treeland");
        QVERIFY(isTreeLandEnvironment());

        qputenv("DDE_CURRENT_COMPOSITOR", "KWin");
        qputenv("DSR_TREELAND_DEMO", "1");
        QVERIFY(isTreeLandEnvironment());
    }

    void annotationContentBoundsCoverGeometryWithoutDecorations()
    {
        Toolshape rectangle;
        rectangle.type = QStringLiteral("rectangle");
        rectangle.lineWidth = 7;
        rectangle.mainPoints = {QPointF(40, 20), QPointF(20, 80),
                                QPointF(100, 40), QPointF(80, 100)};
        const QRectF bounds = toolshapeContentBounds(rectangle);
        QVERIFY(bounds.contains(QPointF(20, 80)));
        QVERIFY(bounds.contains(QPointF(100, 40)));
        QVERIFY(bounds.width() > 80);

        Toolshape pen;
        pen.type = QStringLiteral("pen");
        pen.lineWidth = 3;
        pen.points = {QPointF(10, 10), QPointF(30, 45), QPointF(90, 70)};
        const QRectF penBounds = toolshapeContentBounds(pen);
        QVERIFY(penBounds.contains(QPointF(10, 10)));
        QVERIFY(penBounds.contains(QPointF(90, 70)));

        Toolshape arrowFromOrigin;
        arrowFromOrigin.type = QStringLiteral("arrow");
        arrowFromOrigin.lineWidth = 2;
        arrowFromOrigin.points = {QPointF(0, 0), QPointF(120, 90)};
        const QRectF arrowBounds = toolshapeContentBounds(arrowFromOrigin);
        QVERIFY(arrowBounds.contains(QPointF(0, 0)));
        QVERIFY(arrowBounds.contains(QPointF(120, 90)));

        Toolshape empty;
        QVERIFY(toolshapeContentBounds(empty).isEmpty());
    }

    void effectAnnotationImageProducesStableMosaicBlocks()
    {
        QImage source(64, 64, QImage::Format_ARGB32_Premultiplied);
        source.fill(Qt::red);
        QPainter painter(&source);
        painter.fillRect(QRect(32, 0, 32, 64), Qt::blue);
        painter.end();

        const QImage mosaic = effectAnnotationImage(source, false, 16);

        QCOMPARE(mosaic.size(), source.size());
        QCOMPARE(mosaic.pixelColor(0, 0), mosaic.pixelColor(15, 15));
        QCOMPARE(mosaic.pixelColor(48, 0), mosaic.pixelColor(63, 15));
        QVERIFY(mosaic.pixelColor(0, 0) != mosaic.pixelColor(48, 0));

        const QImage blur = effectAnnotationImage(source, true, 16);
        QCOMPARE(blur.size(), source.size());
        QVERIFY(blur.pixelColor(30, 32) != blur.pixelColor(34, 32));
    }

    void shapeClipGuardPreservesOuterClip()
    {
        QImage target(64, 64, QImage::Format_ARGB32_Premultiplied);
        target.fill(Qt::transparent);
        QPainter painter(&target);
        const QRect outerClip(0, 0, 16, 16);
        painter.setClipRect(outerClip);

        QPainterPath shapePath;
        shapePath.addRect(QRectF(0, 0, 48, 48));
        {
            Dsr::Shapes::ScopedClipPath clip(painter, shapePath);
            QCOMPARE(painter.clipRegion(), QRegion(outerClip));
        }

        // The caller's selection clip must survive annotation painting;
        // otherwise resize handles are drawn outside the selection.
        QCOMPARE(painter.clipRegion(), QRegion(outerClip));
    }

    void roundedCornersPreserveSizeAndDevicePixelRatio()
    {
        QImage source(80, 80, QImage::Format_ARGB32_Premultiplied);
        source.setDevicePixelRatio(2.0);
        source.fill(Qt::red);

        const QImage rounded = Dsr::ImageEffects::roundedCorners(source, 12.0);

        QCOMPARE(rounded.size(), source.size());
        QCOMPARE(rounded.devicePixelRatio(), 2.0);
        QCOMPARE(rounded.pixelColor(0, 0).alpha(), 0);
        QCOMPARE(rounded.pixelColor(40, 40), QColor(Qt::red));
    }

    void whiteCompositionPreservesSizeAndDevicePixelRatio()
    {
        QImage source(40, 20, QImage::Format_ARGB32_Premultiplied);
        source.setDevicePixelRatio(2.0);
        source.fill(Qt::transparent);
        QPainter painter(&source);
        painter.fillRect(QRect(10, 5, 20, 10), Qt::red);
        painter.end();

        const QImage flattened = Dsr::ImageEffects::compositeOnWhite(source);

        QCOMPARE(flattened.size(), source.size());
        QCOMPARE(flattened.devicePixelRatio(), 2.0);
        QCOMPARE(flattened.pixelColor(0, 0), QColor(Qt::white));
        QCOMPARE(flattened.pixelColor(0, 0).alpha(), 255);
        QCOMPARE(flattened.pixelColor(20, 10), QColor(Qt::red));
    }

    void selectionShrinkCanHideAnnotationsOutsideSelection()
    {
        SelectionModel model;
        model.setDesktopBounds(QRect(0, 0, 1000, 800));
        model.setGeometry(QRect(100, 100, 600, 400));
        model.setAnnotationBounds(QRectF(200, 200, 200, 100));

        QCOMPARE(model.beginInteraction(QPoint(699, 300)), SelectionAction::ResizeRight);
        model.updateInteraction(QPoint(250, 300));
        model.endInteraction();

        const QRect result = model.geometry();
        QCOMPARE(result, QRect(100, 100, 151, 400));
        QVERIFY(!result.contains(QPoint(399, 299)));
    }

    void annotationBoundsUpdateKeepsReadySelectionFixed()
    {
        SelectionModel model;
        model.setDesktopBounds(QRect(0, 0, 1000, 800));
        model.setGeometry(QRect(100, 100, 300, 200));
        const QRect original = model.geometry();

        FakeAnnotationEditor editor;
        AnnotationCoordinator coordinator(&model, &editor);
        editor.setBounds(QRectF(750, 150, 100, 100));

        QCOMPARE(model.geometry(), original);
    }

    void annotationBoundsAcrossOutputsKeepReadySelectionFixed()
    {
        SelectionModel model;
        model.setDesktopBounds(QRect(0, 0, 3840, 1080));
        model.setGeometry(QRect(100, 100, 300, 200));
        const QRect original = model.geometry();

        FakeAnnotationEditor editor;
        AnnotationCoordinator coordinator(&model, &editor);
        editor.setBounds(QRectF(100, 100, 2100, 600));

        QCOMPARE(model.geometry(), original);
    }

    void multiScreenOutlineOmitsInternalBoundaries()
    {
        const QRect selection(100, 20, 2200, 140);
        const SelectionOverlayGeometry::Outline left =
                SelectionOverlayGeometry::outlineForOutput(
                        selection, QRect(0, 0, 1920, 1080));
        QCOMPARE(left.localRect, QRect(100, 20, 1820, 140));
        QVERIFY(left.top);
        QVERIFY(left.bottom);
        QVERIFY(left.left);
        QVERIFY(!left.right);

        const SelectionOverlayGeometry::Outline right =
                SelectionOverlayGeometry::outlineForOutput(
                        selection, QRect(1920, 0, 1920, 1080));
        QCOMPARE(right.localRect, QRect(0, 20, 380, 140));
        QVERIFY(right.top);
        QVERIFY(right.bottom);
        QVERIFY(!right.left);
        QVERIFY(right.right);
    }

    void secondOutputBottomOutlineReachesLastScreenPixel()
    {
        const QRect selection(1920, 580, 720, 500);
        const QRect secondOutput(1920, 0, 1920, 1080);
        const QList<QLineF> lines =
                SelectionOverlayGeometry::strokedOutlineForOutput(
                        selection, secondOutput, 2.0);

        QCOMPARE(lines.size(), 4);
        const QLineF bottom = lines.at(2);
        QCOMPARE(bottom.p1(), QPointF(1, 1079));
        QCOMPARE(bottom.p2(), QPointF(719, 1079));
    }

    void strokedOutlineKeepsOnePixelBorderOnScreenBottom()
    {
        const QRect selection(100, 600, 300, 480);
        const QList<QLineF> lines =
                SelectionOverlayGeometry::strokedOutlineForOutput(
                        selection, QRect(0, 0, 1920, 1080), 1.0);

        QCOMPARE(lines.size(), 4);
        QCOMPARE(lines.at(2).p1().y(), 1079.5);
        QCOMPARE(lines.at(2).p2().y(), 1079.5);
    }

    void dashedFrameSitsJustOutsideSelectionLikeX11()
    {
        const QList<QLineF> lines =
                SelectionOverlayGeometry::dashedFrameForOutput(
                        QRect(100, 100, 200, 200), QRect(0, 0, 1920, 1080));

        QCOMPARE(lines.size(), 4);
        // X11 draws at recordX - 1 / recordY - 1 with a 1 px pen, so the
        // dashed frame surrounds the capture region instead of covering its
        // first and last pixel rows.
        QCOMPARE(lines.at(0), QLineF(99, 99, 300, 99));
        QCOMPARE(lines.at(1), QLineF(300, 99, 300, 300));
        QCOMPARE(lines.at(2), QLineF(99, 300, 300, 300));
        QCOMPARE(lines.at(3), QLineF(99, 99, 99, 300));
    }

    void dashedFrameClampsToOutputEdges()
    {
        const QList<QLineF> lines =
                SelectionOverlayGeometry::dashedFrameForOutput(
                        QRect(0, 0, 1920, 1080), QRect(0, 0, 1920, 1080));

        QCOMPARE(lines.size(), 4);
        QCOMPARE(lines.at(0), QLineF(0, 0, 1919, 0));
        QCOMPARE(lines.at(1), QLineF(1919, 0, 1919, 1079));
        QCOMPARE(lines.at(2), QLineF(0, 1079, 1919, 1079));
        QCOMPARE(lines.at(3), QLineF(0, 0, 0, 1079));
    }

    void dashedFrameOmitsInternalOutputBoundary()
    {
        const QRect selection(100, 20, 2200, 140);
        const QList<QLineF> left =
                SelectionOverlayGeometry::dashedFrameForOutput(
                        selection, QRect(0, 0, 1920, 1080));
        QCOMPARE(left.size(), 3);
        QCOMPARE(left.at(0), QLineF(99, 19, 1919, 19));
        QCOMPARE(left.at(1), QLineF(99, 160, 1919, 160));
        QCOMPARE(left.at(2), QLineF(99, 19, 99, 160));

        const QList<QLineF> right =
                SelectionOverlayGeometry::dashedFrameForOutput(
                        selection, QRect(1920, 0, 1920, 1080));
        QCOMPARE(right.size(), 3);
        QCOMPARE(right.at(0), QLineF(0, 19, 380, 19));
        QCOMPARE(right.at(1), QLineF(380, 19, 380, 160));
        QCOMPARE(right.at(2), QLineF(0, 160, 380, 160));
    }

    void crossOutputSnapTargetUsesDominantOutput()
    {
        const QList<QRect> outputs = {
            QRect(0, 0, 1920, 1080),
            QRect(1920, 0, 1920, 1080)
        };
        const QRect resolved = SelectionOverlayGeometry::snapRegionForOutputs(
                QRect(1904, -10, 1952, 1085), outputs);

        QCOMPARE(resolved, QRect(1920, 0, 1920, 1080));
        QVERIFY(!resolved.intersects(outputs.first()));
    }

    void singleOutputSnapTargetKeepsWindowBounds()
    {
        const QList<QRect> outputs = {
            QRect(0, 0, 1920, 1080),
            QRect(1920, 0, 1920, 1080)
        };
        const QRect target(2100, 200, 720, 460);
        const QRect resolved = SelectionOverlayGeometry::snapRegionForOutputs(
                target, outputs);

        QCOMPARE(resolved, target);
    }

    void multiScreenOutlineSupportsVerticalAndNegativeOutputs()
    {
        const QRect selection(-50, -80, 900, 700);
        const SelectionOverlayGeometry::Outline top =
                SelectionOverlayGeometry::outlineForOutput(
                        selection, QRect(-100, -100, 1000, 600));
        QVERIFY(top.top);
        QVERIFY(!top.bottom);
        QVERIFY(top.left);
        QVERIFY(top.right);

        const SelectionOverlayGeometry::Outline bottom =
                SelectionOverlayGeometry::outlineForOutput(
                        selection, QRect(-100, 500, 1000, 600));
        QVERIFY(!bottom.top);
        QVERIFY(bottom.bottom);
        QVERIFY(bottom.left);
        QVERIFY(bottom.right);
    }

    void multiScreenHandlesBelongToOnlyOneVisibleOutput()
    {
        const QRect selection(100, 20, 2200, 140);
        const QList<QPoint> left = SelectionOverlayGeometry::handlesForOutput(
                selection, QRect(0, 0, 1920, 1080));
        const QList<QPoint> right = SelectionOverlayGeometry::handlesForOutput(
                selection, QRect(1920, 0, 1920, 1080));

        QCOMPARE(left.size(), 5);
        QCOMPARE(right.size(), 3);
        QVERIFY(left.contains(QPoint(100, 20)));
        QVERIFY(left.contains(QPoint(100, 159)));
        QVERIFY(left.contains(QPoint(1199, 20)));
        QVERIFY(right.contains(QPoint(379, 20)));
        QVERIFY(right.contains(QPoint(379, 159)));
        QVERIFY(right.contains(QPoint(379, 89)));
    }

    void selectionOwnerUsesLargestOutputIntersection()
    {
        const QList<QRect> outputs = {
            QRect(0, 0, 1920, 1080),
            QRect(1920, 0, 1920, 1080)
        };
        QCOMPARE(SelectionOverlayGeometry::largestSelectionIntersectionIndex(
                         QRect(1000, 0, 2000, 100), outputs),
                 1);
        QCOMPARE(SelectionOverlayGeometry::largestSelectionIntersectionIndex(
                         QRect(100, 100, 500, 500), outputs),
                 0);
        QCOMPARE(SelectionOverlayGeometry::largestSelectionIntersectionIndex(
                         QRect(), outputs),
                 0);
        QCOMPARE(SelectionOverlayGeometry::largestSelectionIntersectionIndex(
                         QRect(0, 0, 10, 10), {}),
                 -1);
    }

    void exportScalesMixedDprOutputsIntoOneLogicalLayout()
    {
        FrozenFrameSet frames;
        frames.virtualDesktopRect = QRect(0, 0, 200, 100);
        frames.outputs = {
            makeFrame(QRect(0, 0, 100, 100), Qt::red, 1.0),
            makeFrame(QRect(100, 0, 100, 100), Qt::blue, 2.0)
        };

        ScreenshotExportService service;
        const Result result = service.render(frames, QRect(50, 25, 100, 50));

        QVERIFY(result.isValid());
        QCOMPARE(result.image.size(), QSize(200, 100));
        QCOMPARE(result.image.pixelColor(99, 50), QColor(Qt::red));
        QCOMPARE(result.image.pixelColor(100, 50), QColor(Qt::blue));
        QCOMPARE(result.image.pixelColor(199, 50), QColor(Qt::blue));
    }

    void exportJoinsOutputsAndRendersAnnotations()
    {
        FrozenFrameSet frames;
        frames.virtualDesktopRect = QRect(0, 0, 3840, 1080);
        frames.generation = 42;
        frames.outputs = {
            makeFrame(QRect(0, 0, 1920, 1080), Qt::red),
            makeFrame(QRect(1920, 0, 1920, 1080), Qt::blue)
        };

        FakeAnnotationEditor editor;
        editor.renderColor = Qt::green;
        ScreenshotExportService service;
        const Result result = service.render(frames, QRect(1800, 0, 300, 100), &editor);

        QVERIFY(result.isValid());
        QCOMPARE(result.image.size(), QSize(300, 100));
        QCOMPARE(result.image.pixelColor(10, 50), QColor(Qt::green));
        QCOMPARE(result.image.pixelColor(290, 50), QColor(Qt::green));
        QCOMPARE(result.frameGeneration, quint64(42));
        QVERIFY(editor.renderCalled);
    }

    void controllerDrivesSaveThroughPorts()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};

        auto *overlay = new FakeOverlay;
        auto *snapHints = new FakeSnapHints;
        auto *tagger = new FakeWindowTagger;
        auto *sink = new FakeResultSink;
        auto *annotations = new FakeAnnotationEditor;
        annotations->bounds = QRectF(400, 250, 100, 100);
        annotations->renderColor = Qt::green;

        ScreenshotSessionController controller(backend,
                                               overlay,
                                               snapHints,
                                               tagger,
                                               sink,
                                               annotations);
        QSignalSpy savedSpy(&controller, &ScreenshotSessionController::saved);

        Request request;
        request.outputPath = QStringLiteral("captured.png");
        request.enableWindowSnap = false;
        QVERIFY(controller.start(request));
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Selecting);
        QCOMPARE(overlay->createdOutputs.size(), 1);

        overlay->press(QPoint(100, 100));
        overlay->move(QPoint(500, 400));
        overlay->release(QPoint(500, 400));
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Editing);
        QVERIFY(overlay->selection.right() >= 499);
        QVERIFY(overlay->finalized);

        overlay->confirm();
        QCOMPARE(savedSpy.count(), 1);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
        QCOMPARE(sink->saveCount, 1);
        QCOMPARE(sink->requestedPath, QStringLiteral("captured.png"));
        QVERIFY(sink->lastResult.isValid());
        QCOMPARE(sink->lastResult.image.pixelColor(0, 0), QColor(Qt::green));
        QVERIFY(sink->notificationsEnabled);
        QCOMPARE(sink->notificationSettingCount, 1);
        QVERIFY(overlay->closed);
    }

    void recordModeUsesRecorderToolbarFromStart()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        Request request;
        request.mode = Mode::Record;
        request.enableWindowSnap = false;

        QVERIFY(controller.start(request));
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Selecting);
        QVERIFY(overlay->hasInitialMode);
        QCOMPARE(overlay->initialMode, Mode::Record);
        QVERIFY(overlay->recordModeActive);
    }

    void switchingToRecordClearsScreenshotAnnotations()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *annotations = new FakeAnnotationEditor;

        ScreenshotSessionController controller(backend,
                                               overlay,
                                               nullptr,
                                               nullptr,
                                               sink,
                                               annotations);
        Request request;
        request.enableWindowSnap = false;

        QVERIFY(controller.start(request));
        overlay->press(QPoint(100, 100));
        overlay->move(QPoint(500, 400));
        overlay->release(QPoint(500, 400));
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Editing);

        annotations->setBounds(QRectF(200, 200, 80, 60));
        overlay->requestRecordMode(true);

        QCOMPARE(annotations->clearCount, 1);
        QVERIFY(annotations->bounds.isEmpty());
        QVERIFY(overlay->recordModeActive);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Editing);
    }

    void recordFullScreenStartsCountdownAndPassesOutputPath()
    {
        QScreen *screen = QGuiApplication::primaryScreen();
        QVERIFY(screen);

        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = screen->geometry();
        backend->frames.outputs = {
            makeFrame(screen->geometry(), Qt::red)
        };
        backend->frames.outputs.first().output.outputId = screen->name();

        auto *overlay = new FakeOverlay;
        overlay->emitCountdownImmediately = true;
        auto *recording = new FakeRecordingSession;
        auto *sink = new FakeResultSink;

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        controller.setRecordingSession(recording);

        Request request;
        request.mode = Mode::RecordFullScreen;
        request.enableWindowSnap = false;
        request.outputPath = QStringLiteral("/tmp/treeland-record.mp4");

        QVERIFY(controller.start(request));
        QTRY_COMPARE_WITH_TIMEOUT(controller.state(),
                                  ScreenshotSessionController::State::Countdown,
                                  2000);
        QCOMPARE(overlay->selection, screen->geometry());
        QCOMPARE(overlay->initialMode, Mode::RecordFullScreen);
        QCOMPARE(overlay->countdownStartCount, 1);

        QTRY_COMPARE_WITH_TIMEOUT(recording->startCount, 1, 2000);
        QCOMPARE(recording->lastRequest.outputPath,
                 QStringLiteral("/tmp/treeland-record.mp4"));
        QCOMPARE(recording->lastRequest.format, int(Utils::kMP4));
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Recording);
    }

    void scrollModeAutomaticallyStartsAfterSelection()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *capture = new FakeScrollCapture;
        auto *merger = new FakeScrollMerger;

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        controller.setScrollCapture(capture);
        controller.setScrollMerger(merger);

        Request request;
        request.mode = Mode::Scroll;
        request.enableWindowSnap = false;

        QVERIFY(controller.start(request));
        overlay->press(QPoint(100, 100));
        overlay->move(QPoint(299, 299));
        overlay->release(QPoint(299, 299));

        QTRY_COMPARE_WITH_TIMEOUT(capture->startCount, 1, 2000);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Scrolling);
        QCOMPARE(overlay->scrollSelection, QRect(100, 100, 200, 200));
        QVERIFY(overlay->scrollActive);
    }

    void ocrModeAutomaticallyConsumesSelection()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        Request request;
        request.mode = Mode::Ocr;
        request.enableWindowSnap = false;

        QVERIFY(controller.start(request));
        overlay->press(QPoint(100, 100));
        overlay->move(QPoint(299, 299));
        overlay->release(QPoint(299, 299));

        QTRY_COMPARE_WITH_TIMEOUT(sink->ocrCount, 1, 2000);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
        QVERIFY(sink->lastResult.isValid());
    }

    void topWindowModeUsesSnapRegionWithoutSecondDrag()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};
        auto *overlay = new FakeOverlay;
        auto *snapHints = new FakeSnapHints;
        auto *sink = new FakeResultSink;

        ScreenshotSessionController controller(backend, overlay, snapHints, nullptr, sink);
        Request request;
        request.mode = Mode::TopWindow;
        // The fake overlay cannot tag native mask windows. Snap events still
        // reach the controller through ISnapHintSource, which is the part
        // this test exercises.
        request.enableWindowSnap = false;

        QVERIFY(controller.start(request));
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Selecting);

        snapHints->setRegion(QRect(120, 80, 320, 240));
        QTRY_COMPARE_WITH_TIMEOUT(sink->saveCount, 1, 2000);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
        QCOMPARE(overlay->selection, QRect(120, 80, 320, 240));
    }

    void delayedSessionWaitsBeforePreparingBackend()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        Request request;
        request.delayMs = 80;

        QVERIFY(controller.start(request));
        QCOMPARE(controller.state(), ScreenshotSessionController::State::PreparingFrame);
        QCOMPARE(backend->prepareCount, 0);

        QTRY_COMPARE_WITH_TIMEOUT(backend->prepareCount, 1, 2000);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Selecting);
    }

    void cancelledDelayDoesNotPrepareBackend()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        Request request;
        request.delayMs = 300;

        QVERIFY(controller.start(request));
        controller.cancel();
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Cancelled);
        QTest::qWait(450);
        QCOMPARE(backend->prepareCount, 0);
    }

    void scrollInputRegionKeepsInteractiveControlsAndPassesWheelThroughHole()
    {
        const QRect surface(0, 0, 800, 600);
        OutputDescriptor output;
        output.logicalGeometry = QRect(0, 0, 800, 600);
        const QRect selection(200, 100, 300, 200);
        const QRect toolBar(520, 320, 225, 68);

        const QRegion region = scrollInputRegion(
                surface, output, selection, {toolBar});

        QVERIFY(region.contains(QPoint(10, 10)));
        QVERIFY(!region.contains(QPoint(300, 150)));
        QVERIFY(region.contains(QPoint(540, 340)));
    }

    void scrollInputRegionRestoresToolBarInsideSelection()
    {
        const QRect surface(0, 0, 800, 600);
        OutputDescriptor output;
        output.logicalGeometry = QRect(0, 0, 800, 600);
        const QRect selection(100, 100, 400, 300);
        const QRect toolBar(180, 220, 225, 68);

        const QRegion region = scrollInputRegion(
                surface, output, selection, {toolBar});

        QVERIFY(!region.contains(QPoint(120, 120)));
        QVERIFY(region.contains(QPoint(200, 240)));
    }

    void scrollInputRegionConvertsGlobalSelectionPerOutput()
    {
        const QRect surface(0, 0, 1920, 1080);
        OutputDescriptor output;
        output.logicalGeometry = QRect(1920, 0, 1920, 1080);
        const QRect selection(2000, 100, 200, 200);

        const QRegion region = scrollInputRegion(
                surface, output, selection, {});

        QVERIFY(region.contains(QPoint(10, 10)));
        QVERIFY(!region.contains(QPoint(90, 110)));
        QVERIFY(region.contains(QPoint(500, 500)));
    }

    void scrollTipPositionMatchesX11PlacementRules()
    {
        const QSize surface(800, 600);
        const QRect output(0, 0, 800, 600);
        const QSize tip(200, 40);

        // X11 centers prompts on the screen for selections smaller than
        // 300x100.
        QCOMPARE(scrollTipPosition(surface,
                                   QRect(100, 100, 200, 90),
                                   output,
                                   QRect(100, 20, 225, 68),
                                   tip),
                 QPoint(300, 280));

        // Toolbar inside the selection: prompt goes below the toolbar.
        QCOMPARE(scrollTipPosition(surface,
                                   QRect(100, 100, 400, 300),
                                   output,
                                   QRect(200, 250, 225, 68),
                                   tip),
                 QPoint(200, 333));

        // Toolbar below the selection: prompt goes above the toolbar.
        QCOMPARE(scrollTipPosition(surface,
                                   QRect(100, 100, 400, 300),
                                   output,
                                   QRect(200, 420, 225, 68),
                                   tip),
                 QPoint(200, 365));

        // Toolbar above the selection: prompt goes below the toolbar.
        QCOMPARE(scrollTipPosition(surface,
                                   QRect(100, 100, 400, 300),
                                   output,
                                   QRect(200, 20, 225, 68),
                                   tip),
                 QPoint(200, 103));

        // Selection coordinates are global while the toolbar is window-local.
        QCOMPARE(scrollTipPosition(surface,
                                   QRect(2020, 100, 400, 300),
                                   QRect(1920, 0, 800, 600),
                                   QRect(200, 420, 225, 68),
                                   tip),
                 QPoint(200, 365));

        // The X11 300x100 threshold is in physical pixels. At 2x scale a
        // 200x90 logical selection uses the selection-relative placement.
        QCOMPARE(scrollTipPosition(surface,
                                   QRect(100, 100, 200, 90),
                                   output,
                                   QRect(300, 200, 225, 68),
                                   tip,
                                   2.0),
                 QPoint(100, 145));
    }

    void controllerStartsScrollAndSavesLongImage()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *capture = new FakeScrollCapture;
        auto *merger = new FakeScrollMerger;
        merger->result = QImage(200, 420, QImage::Format_ARGB32_Premultiplied);
        merger->result.fill(Qt::blue);

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        controller.setScrollCapture(capture);
        controller.setScrollMerger(merger);
        QSignalSpy savedSpy(&controller, &ScreenshotSessionController::saved);

        Request request;
        request.mode = Mode::Scroll;
        request.enableWindowSnap = false;
        request.initialRegion = QRect(100, 100, 200, 200);
        request.outputPath = QStringLiteral("scroll.png");

        QVERIFY(controller.start(request));
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Selecting);
        QVERIFY(overlay->finalized);

        overlay->scrollStart();
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Scrolling);
        QCOMPARE(overlay->scrollBeginCount, 1);
        QCOMPARE(overlay->scrollSelection, QRect(100, 100, 200, 200));
        QCOMPARE(capture->startCount, 1);
        QCOMPARE(capture->selection, QRect(100, 100, 200, 200));
        QCOMPARE(merger->startCount, 1);

        ScrollFrame frame;
        frame.output = backend->frames.outputs.first().output;
        frame.globalSelection = QRect(100, 100, 200, 200);
        frame.image = QImage(200, 200, QImage::Format_ARGB32_Premultiplied);
        frame.image.fill(Qt::green);
        capture->emitFrame(frame);
        QCOMPARE(merger->addCount, 1);
        capture->replayFrameOnRequest = true;

        overlay->scrollStop();
        QCOMPARE(capture->requestCount, 1);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
        QCOMPARE(merger->finishCount, 1);
        QCOMPARE(sink->saveCount, 1);
        QCOMPARE(sink->requestedPath, QStringLiteral("scroll.png"));
        QCOMPARE(sink->lastResult.image, merger->result);
        QCOMPARE(sink->lastResult.globalSelection, QRect(100, 100, 200, 420));
        QCOMPARE(savedSpy.count(), 1);
        QVERIFY(capture->stopCount >= 1);
        QVERIFY(overlay->scrollFinishCount >= 1);
        // The export path must not restore the screenshot mask: doing so
        // flashes the frozen frame and rounded corners before the result UI.
        QCOMPARE(overlay->scrollFinishRestoreArgs.first(), false);
        QVERIFY(overlay->closed);
    }

    void controllerSavesStitchedImageWhenFinalScrollFrameFails()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *capture = new FakeScrollCapture;
        auto *merger = new FakeScrollMerger;
        merger->result = QImage(200, 420, QImage::Format_ARGB32_Premultiplied);
        merger->result.fill(Qt::blue);

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        controller.setScrollCapture(capture);
        controller.setScrollMerger(merger);
        QSignalSpy savedSpy(&controller, &ScreenshotSessionController::saved);
        QSignalSpy failedSpy(&controller, &ScreenshotSessionController::failed);

        Request request;
        request.mode = Mode::Scroll;
        request.enableWindowSnap = false;
        request.initialRegion = QRect(100, 100, 200, 200);
        request.outputPath = QStringLiteral("scroll-fallback.png");

        QVERIFY(controller.start(request));
        overlay->scrollStart();
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Scrolling);

        ScrollFrame frame;
        frame.output = backend->frames.outputs.first().output;
        frame.globalSelection = QRect(100, 100, 200, 200);
        frame.image = QImage(200, 200, QImage::Format_ARGB32_Premultiplied);
        frame.image.fill(Qt::green);
        capture->emitFrame(frame);
        QCOMPARE(merger->addCount, 1);

        capture->failOnRequest = true;
        overlay->scrollStop();

        QCOMPARE(capture->pauseCount, 1);
        QCOMPARE(capture->requestCount, 1);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
        QCOMPARE(failedSpy.count(), 0);
        QCOMPARE(savedSpy.count(), 1);
        QCOMPARE(merger->finishCount, 1);
        QCOMPARE(sink->saveCount, 1);
        QCOMPARE(sink->requestedPath, QStringLiteral("scroll-fallback.png"));
        QCOMPARE(sink->lastResult.image, merger->result);
        QCOMPARE(sink->lastResult.globalSelection, QRect(100, 100, 200, 420));
        QVERIFY(overlay->closed);
    }

    void controllerKeepsScrollSessionAfterRecoverableFrameTimeouts()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *capture = new FakeScrollCapture;
        auto *merger = new FakeScrollMerger;
        merger->result = QImage(200, 420, QImage::Format_ARGB32_Premultiplied);
        merger->result.fill(Qt::blue);

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        controller.setScrollCapture(capture);
        controller.setScrollMerger(merger);
        QSignalSpy failedSpy(&controller, &ScreenshotSessionController::failed);
        QSignalSpy savedSpy(&controller, &ScreenshotSessionController::saved);

        Request request;
        request.mode = Mode::Scroll;
        request.enableWindowSnap = false;
        request.initialRegion = QRect(100, 100, 200, 200);
        request.outputPath = QStringLiteral("scroll-timeout.png");

        QVERIFY(controller.start(request));
        overlay->scrollStart();
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Scrolling);

        ScrollFrame frame;
        frame.output = backend->frames.outputs.first().output;
        frame.globalSelection = QRect(100, 100, 200, 200);
        frame.image = QImage(200, 200, QImage::Format_ARGB32_Premultiplied);
        frame.image.fill(Qt::green);
        capture->emitFrame(frame);

        capture->failFrame(QStringLiteral("等待输出截图帧超时：等待 Wayland 事件超时"));
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Scrolling);
        // One missed frame is retried silently; it is not an X11 merge error
        // and must not flash the error prompt.
        QCOMPARE(overlay->lastScrollTip, ScrollTipKind::Start);
        QCOMPARE(capture->pauseCount, 0);
        capture->failFrame(QStringLiteral("等待输出截图帧超时：等待 Wayland 事件超时"));

        // Repeated failures pause capture, display the same error prompt as
        // X11, and keep the result saveable instead of closing the session.
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Scrolling);
        QCOMPARE(failedSpy.count(), 0);
        QVERIFY(!overlay->closed);
        QCOMPARE(overlay->lastScrollTip, ScrollTipKind::Error);
        QCOMPARE(capture->pauseCount, 1);

        overlay->scrollStop();
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
        QCOMPARE(savedSpy.count(), 1);
        QCOMPARE(sink->saveCount, 1);
        QVERIFY(overlay->closed);
    }

    void controllerShowsQuickScrollTipWithoutPausingCapture()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *capture = new FakeScrollCapture;
        auto *merger = new FakeScrollMerger;
        merger->result = QImage(200, 300, QImage::Format_ARGB32_Premultiplied);
        merger->result.fill(Qt::blue);

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        controller.setScrollCapture(capture);
        controller.setScrollMerger(merger);

        Request request;
        request.mode = Mode::Scroll;
        request.enableWindowSnap = false;
        request.initialRegion = QRect(100, 100, 200, 200);
        QVERIFY(controller.start(request));
        overlay->scrollStart();

        merger->emitQuickScroll();
        QCOMPARE(overlay->lastScrollTip, ScrollTipKind::QuickScroll);
        QCOMPARE(capture->pauseCount, 0);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Scrolling);
        QVERIFY(!overlay->closed);
    }

    void controllerShowsEndTipAndKeepsSessionSaveable()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *capture = new FakeScrollCapture;
        auto *merger = new FakeScrollMerger;
        merger->result = QImage(200, 300, QImage::Format_ARGB32_Premultiplied);
        merger->result.fill(Qt::blue);

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        controller.setScrollCapture(capture);
        controller.setScrollMerger(merger);
        QSignalSpy failedSpy(&controller, &ScreenshotSessionController::failed);
        QSignalSpy savedSpy(&controller, &ScreenshotSessionController::saved);

        Request request;
        request.mode = Mode::Scroll;
        request.enableWindowSnap = false;
        request.initialRegion = QRect(100, 100, 200, 200);
        request.outputPath = QStringLiteral("scroll-bottom.png");
        QVERIFY(controller.start(request));
        overlay->scrollStart();

        merger->emitReachedBottom();
        QCOMPARE(overlay->lastScrollTip, ScrollTipKind::End);
        QCOMPARE(capture->pauseCount, 1);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Scrolling);
        QCOMPARE(failedSpy.count(), 0);

        overlay->scrollStop();
        QCOMPARE(savedSpy.count(), 1);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
    }

    void controllerShowsInvalidAreaTipAndKeepsSessionSaveable()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *capture = new FakeScrollCapture;
        auto *merger = new FakeScrollMerger;
        merger->result = QImage(200, 300, QImage::Format_ARGB32_Premultiplied);
        merger->result.fill(Qt::blue);

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        controller.setScrollCapture(capture);
        controller.setScrollMerger(merger);
        QSignalSpy failedSpy(&controller, &ScreenshotSessionController::failed);

        Request request;
        request.mode = Mode::Scroll;
        request.enableWindowSnap = false;
        request.initialRegion = QRect(100, 100, 200, 200);
        QVERIFY(controller.start(request));
        overlay->scrollStart();

        merger->emitInvalidArea();
        QCOMPARE(overlay->lastScrollTip, ScrollTipKind::InvalidArea);
        QCOMPARE(capture->pauseCount, 1);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Scrolling);
        QCOMPARE(failedSpy.count(), 0);
        QVERIFY(!overlay->closed);
    }

    void controllerKeepsSessionAliveWhenScrollMergeFails()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *capture = new FakeScrollCapture;
        auto *merger = new FakeScrollMerger;
        merger->result = QImage(200, 300, QImage::Format_ARGB32_Premultiplied);
        merger->result.fill(Qt::blue);

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        controller.setScrollCapture(capture);
        controller.setScrollMerger(merger);
        QSignalSpy failedSpy(&controller, &ScreenshotSessionController::failed);
        QSignalSpy savedSpy(&controller, &ScreenshotSessionController::saved);

        Request request;
        request.mode = Mode::Scroll;
        request.enableWindowSnap = false;
        request.initialRegion = QRect(100, 100, 200, 200);
        request.outputPath = QStringLiteral("scroll-merge-error.png");
        QVERIFY(controller.start(request));
        overlay->scrollStart();

        merger->fail(QStringLiteral("滚动截图拼接失败"));
        QCOMPARE(overlay->lastScrollTip, ScrollTipKind::Error);
        QCOMPARE(capture->pauseCount, 1);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Scrolling);
        QCOMPARE(failedSpy.count(), 0);
        QVERIFY(!overlay->closed);

        overlay->scrollStop();
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
        QCOMPARE(savedSpy.count(), 1);
    }

    void controllerSavesLastPreviewWhenMergeResultIsEmpty()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *capture = new FakeScrollCapture;
        auto *merger = new FakeScrollMerger;
        merger->result = QImage();

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        controller.setScrollCapture(capture);
        controller.setScrollMerger(merger);
        QSignalSpy failedSpy(&controller, &ScreenshotSessionController::failed);
        QSignalSpy savedSpy(&controller, &ScreenshotSessionController::saved);

        Request request;
        request.mode = Mode::Scroll;
        request.enableWindowSnap = false;
        request.initialRegion = QRect(100, 100, 200, 200);
        request.outputPath = QStringLiteral("scroll-preview-fallback.png");
        QVERIFY(controller.start(request));
        overlay->scrollStart();

        QImage preview(200, 260, QImage::Format_ARGB32_Premultiplied);
        preview.fill(Qt::magenta);
        merger->emitPreview(preview);
        merger->fail(QStringLiteral("滚动截图拼接失败"));

        QCOMPARE(controller.state(), ScreenshotSessionController::State::Scrolling);
        QCOMPARE(failedSpy.count(), 0);
        QCOMPARE(sink->saveCount, 0);

        overlay->scrollStop();
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
        QCOMPARE(savedSpy.count(), 1);
        QCOMPARE(sink->lastResult.image, preview);
    }

    void controllerScrollConfirmCopiesAndSaveButtonSaves()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *capture = new FakeScrollCapture;
        auto *merger = new FakeScrollMerger;
        merger->result = QImage(200, 300, QImage::Format_ARGB32_Premultiplied);
        merger->result.fill(Qt::blue);

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        controller.setScrollCapture(capture);
        controller.setScrollMerger(merger);
        QSignalSpy copiedSpy(&controller, &ScreenshotSessionController::copied);
        QSignalSpy savedSpy(&controller, &ScreenshotSessionController::saved);

        Request request;
        request.mode = Mode::Scroll;
        request.enableWindowSnap = false;
        request.initialRegion = QRect(100, 100, 200, 200);
        request.outputPath = QStringLiteral("scroll-confirm.png");

        QVERIFY(controller.start(request));
        overlay->scrollStart();
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Scrolling);

        capture->replayFrameOnRequest = true;
        overlay->copy();

        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
        QCOMPARE(capture->requestCount, 1);
        QCOMPARE(sink->copyCount, 1);
        QCOMPARE(sink->saveCount, 0);
        QCOMPARE(sink->lastResult.image, merger->result);
        QCOMPARE(copiedSpy.count(), 1);
        QCOMPARE(savedSpy.count(), 0);

        FakeBackend *saveBackend = new FakeBackend;
        saveBackend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        saveBackend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};
        auto *saveOverlay = new FakeOverlay;
        auto *saveSink = new FakeResultSink;
        auto *saveCapture = new FakeScrollCapture;
        auto *saveMerger = new FakeScrollMerger;
        saveMerger->result = merger->result;
        ScreenshotSessionController saveController(saveBackend, saveOverlay,
                                                   nullptr, nullptr, saveSink);
        saveController.setScrollCapture(saveCapture);
        saveController.setScrollMerger(saveMerger);

        Request saveRequest = request;
        QVERIFY(saveController.start(saveRequest));
        saveOverlay->scrollStart();
        saveCapture->replayFrameOnRequest = true;
        saveOverlay->confirm();

        QCOMPARE(saveController.state(), ScreenshotSessionController::State::Completed);
        QCOMPARE(saveCapture->requestCount, 1);
        QCOMPARE(saveSink->saveCount, 1);
        QCOMPARE(saveSink->copyCount, 0);
        QCOMPARE(saveSink->requestedPath, QStringLiteral("scroll-confirm.png"));
    }

    void controllerScrollPreviewIsForwardedToOverlay()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *capture = new FakeScrollCapture;
        auto *merger = new FakeScrollMerger;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        controller.setScrollCapture(capture);
        controller.setScrollMerger(merger);

        Request request;
        request.mode = Mode::Scroll;
        request.enableWindowSnap = false;
        request.initialRegion = QRect(100, 100, 200, 200);
        QVERIFY(controller.start(request));
        overlay->scrollStart();

        QImage preview(200, 420, QImage::Format_ARGB32_Premultiplied);
        preview.fill(Qt::magenta);
        merger->emitPreview(preview);
        QCOMPARE(overlay->scrollPreviewCount, 1);
        QCOMPARE(overlay->lastScrollPreview, preview);

        overlay->cancel();
        merger->emitPreview(preview);
        QCOMPARE(overlay->scrollPreviewCount, 1);
    }

    void controllerGuardsScrollFramesAgainstOverlayWidgets()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *capture = new FakeScrollCapture;
        auto *merger = new FakeScrollMerger;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        controller.setScrollCapture(capture);
        controller.setScrollMerger(merger);

        Request request;
        request.mode = Mode::Scroll;
        request.enableWindowSnap = false;
        request.initialRegion = QRect(100, 100, 200, 200);
        QVERIFY(controller.start(request));
        overlay->scrollStart();

        capture->emitCaptureAboutToStart();
        capture->emitCaptureFinished();
        QCOMPARE(overlay->scrollFrameCaptureBeginCount, 1);
        QCOMPARE(overlay->scrollFrameCaptureEndCount, 1);

        overlay->cancel();
        capture->emitCaptureAboutToStart();
        capture->emitCaptureFinished();
        QCOMPARE(overlay->scrollFrameCaptureBeginCount, 1);
        QCOMPARE(overlay->scrollFrameCaptureEndCount, 1);
    }

    void controllerScrollReachedBottomKeepsPreviewForConfirmation()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *capture = new FakeScrollCapture;
        auto *merger = new FakeScrollMerger;
        merger->result = QImage(200, 420, QImage::Format_ARGB32_Premultiplied);
        merger->result.fill(Qt::cyan);
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        controller.setScrollCapture(capture);
        controller.setScrollMerger(merger);

        Request request;
        request.mode = Mode::Scroll;
        request.enableWindowSnap = false;
        request.initialRegion = QRect(100, 100, 200, 200);
        QVERIFY(controller.start(request));
        overlay->scrollStart();

        merger->emitReachedBottom();
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Scrolling);
        QVERIFY(overlay->scrollActive);
        QVERIFY(capture->pauseCount >= 1);
        QCOMPARE(capture->requestCount, 0);
        QCOMPARE(sink->saveCount, 0);
        QCOMPARE(sink->copyCount, 0);

        overlay->copy();
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
        QCOMPARE(capture->requestCount, 0);
        QCOMPARE(sink->copyCount, 1);
        QCOMPARE(sink->lastResult.image, merger->result);
    }

    void scrollCompositorStitchesAdjacentOutputs()
    {
        const OutputDescriptor left{QStringLiteral("left"), QRect(0, 0, 1920, 1080), 1.0};
        const OutputDescriptor right{QStringLiteral("right"), QRect(1920, 0, 1920, 1080), 1.0};
        QImage leftImage(1920, 1080, QImage::Format_RGB32);
        leftImage.fill(Qt::red);
        QImage rightImage(1920, 1080, QImage::Format_RGB32);
        rightImage.fill(Qt::green);

        const QImage composed = composeScrollSelectionFrame(
                {left, right}, {leftImage, rightImage}, QRect(1800, 0, 240, 200));

        QCOMPARE(composed.size(), QSize(240, 200));
        QCOMPARE(composed.pixelColor(60, 100), QColor(Qt::red));
        QCOMPARE(composed.pixelColor(180, 100), QColor(Qt::green));
    }

    void scrollCompositorHandlesNegativeOrigin()
    {
        const OutputDescriptor left{QStringLiteral("left"), QRect(-1920, 0, 1920, 1080), 1.0};
        const OutputDescriptor right{QStringLiteral("right"), QRect(0, 0, 1920, 1080), 1.0};
        QImage leftImage(1920, 1080, QImage::Format_RGB32);
        leftImage.fill(Qt::red);
        QImage rightImage(1920, 1080, QImage::Format_RGB32);
        rightImage.fill(Qt::green);

        const QImage composed = composeScrollSelectionFrame(
                {left, right}, {leftImage, rightImage}, QRect(-100, 0, 200, 100));

        QCOMPARE(composed.size(), QSize(200, 100));
        QCOMPARE(composed.pixelColor(50, 50), QColor(Qt::red));
        QCOMPARE(composed.pixelColor(150, 50), QColor(Qt::green));
    }

    void scrollCompositorScalesMixedDpr()
    {
        const OutputDescriptor left{QStringLiteral("left"), QRect(0, 0, 1920, 1080), 1.0};
        const OutputDescriptor hidpi{QStringLiteral("hidpi"), QRect(1920, 0, 1280, 720), 1.5};
        QImage leftImage(1920, 1080, QImage::Format_RGB32);
        leftImage.fill(Qt::red);
        QImage hidpiImage(1920, 1080, QImage::Format_RGB32);
        hidpiImage.fill(Qt::green);

        const QImage composed = composeScrollSelectionFrame(
                {left, hidpi}, {leftImage, hidpiImage}, QRect(1900, 0, 340, 200));

        QCOMPARE(composed.size(), QSize(510, 300));
        QCOMPARE(composed.pixelColor(10, 150), QColor(Qt::red));
        QCOMPARE(composed.pixelColor(400, 150), QColor(Qt::green));
    }

    void controllerAcceptsCrossOutputScrollSelection()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 1600, 600);
        backend->frames.outputs = {
            makeFrame(QRect(0, 0, 800, 600), Qt::red),
            makeFrame(QRect(800, 0, 800, 600), Qt::green)
        };

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *capture = new FakeScrollCapture;
        auto *merger = new FakeScrollMerger;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        controller.setScrollCapture(capture);
        controller.setScrollMerger(merger);

        Request request;
        request.mode = Mode::Scroll;
        request.enableWindowSnap = false;
        request.initialRegion = QRect(700, 100, 200, 200);
        QSignalSpy failedSpy(&controller, &ScreenshotSessionController::failed);

        QVERIFY(controller.start(request));
        overlay->scrollStart();

        // Cross-output selections are composed from every intersecting output;
        // the capture backend decides how to stitch them into one frame.
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Scrolling);
        QCOMPARE(failedSpy.count(), 0);
        QCOMPARE(overlay->scrollBeginCount, 1);
        QCOMPARE(capture->startCount, 1);
        QCOMPARE(capture->outputs.size(), 2);
        QCOMPARE(capture->selection, QRect(700, 100, 200, 200));
    }

    void controllerCancelsScrollingWithoutSaving()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *capture = new FakeScrollCapture;
        auto *merger = new FakeScrollMerger;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        controller.setScrollCapture(capture);
        controller.setScrollMerger(merger);

        Request request;
        request.mode = Mode::Scroll;
        request.enableWindowSnap = false;
        request.initialRegion = QRect(100, 100, 200, 200);
        QVERIFY(controller.start(request));
        overlay->scrollStart();
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Scrolling);

        overlay->cancel();

        QCOMPARE(controller.state(), ScreenshotSessionController::State::Cancelled);
        QCOMPARE(sink->saveCount, 0);
        QVERIFY(capture->stopCount >= 1);
        QVERIFY(merger->cancelCount >= 1);
    }


    void pixMergeAutoDetectsDownwardScroll()
    {
        const QImage source = makeScrollSource(120, 600);
        PixMergeThread merge;
        merge.setScrollModel(true);
        merge.addShotImg(scrollViewport(source, 0, 200),
                         PixMergeThread::PictureDirection::Auto);
        merge.start();

        for (int top : {40, 80, 120}) {
            merge.addShotImg(scrollViewport(source, top, 200),
                             PixMergeThread::PictureDirection::Auto);
            QVERIFY(waitForMergeQueue(merge, 0));
        }

        merge.setIsLastImg(true);
        merge.stopTask();
        QVERIFY(merge.wait(5000));
        const QImage result = merge.getMerageResult();
        QCOMPARE(result.size(), QSize(120, 320));
    }

    void pixMergeAutoDetectsUpwardScroll()
    {
        const QImage source = makeScrollSource(120, 600);
        PixMergeThread merge;
        merge.setScrollModel(true);
        merge.addShotImg(scrollViewport(source, 200, 200),
                         PixMergeThread::PictureDirection::Auto);
        merge.start();

        for (int top : {160, 120}) {
            merge.addShotImg(scrollViewport(source, top, 200),
                             PixMergeThread::PictureDirection::Auto);
            QVERIFY(waitForMergeQueue(merge, 0));
        }

        merge.setIsLastImg(true);
        merge.stopTask();
        QVERIFY(merge.wait(5000));
        const QImage result = merge.getMerageResult();
        QCOMPARE(result.size(), QSize(120, 280));
    }


    void pixMergeDropsOldestQueuedFrame()
    {
        const QImage source = makeScrollSource(120, 600);
        PixMergeThread merge;
        merge.setScrollModel(true);
        merge.addShotImg(scrollViewport(source, 0, 200),
                         PixMergeThread::PictureDirection::Auto);
        merge.addShotImg(scrollViewport(source, 40, 200),
                         PixMergeThread::PictureDirection::Auto);
        merge.addShotImg(scrollViewport(source, 80, 200),
                         PixMergeThread::PictureDirection::Auto);
        merge.addShotImg(scrollViewport(source, 120, 200),
                         PixMergeThread::PictureDirection::Auto);

        QCOMPARE(merge.pendingImageCount(), 3);
        QVERIFY(merge.dropOldestPending());
        QCOMPARE(merge.pendingImageCount(), 2);
    }

    void scrollMergeCoordinatorDropsIdenticalFrames()
    {
        const QImage source = makeScrollSource(120, 600);
        ScrollMergeCoordinator merger;
        merger.start(QRect(0, 0, 120, 200));

        ScrollFrame frame;
        frame.image = source.copy(0, 0, 120, 200);
        QVERIFY(merger.addFrame(frame));
        QVERIFY(!merger.addFrame(frame));

        const QImage result = merger.finish();
        QCOMPARE(result.size(), QSize(120, 200));
    }

    void scrollMergeCoordinatorEmitsPreviewForFirstFrame()
    {
        ScrollMergeCoordinator merger;
        QSignalSpy previewSpy(&merger, &IScrollImageMerger::previewUpdated);
        merger.start(QRect(0, 0, 120, 200));

        ScrollFrame frame;
        frame.image = QImage(120, 200, QImage::Format_ARGB32_Premultiplied);
        frame.image.fill(Qt::darkGreen);
        QVERIFY(merger.addFrame(frame));

        QCOMPARE(previewSpy.count(), 1);
        QCOMPARE(previewSpy.first().at(0).value<QImage>(), frame.image);
        merger.cancel();
    }

    void pixMergeManualModeIgnoresIdleFrames()
    {
        PixMergeThread merge;
        merge.setScrollModel(true);
        QSignalSpy errorSpy(&merge, &PixMergeThread::merageError);

        auto frame = [](const QColor &color) {
            QImage image(120, 200, QImage::Format_ARGB32_Premultiplied);
            image.fill(color);
            return QPixmap::fromImage(image);
        };

        merge.addShotImg(frame(Qt::red), PixMergeThread::PictureDirection::Auto);
        merge.start();
        for (const QColor &color : {QColor(Qt::green), QColor(Qt::blue),
                                    QColor(Qt::yellow), QColor(Qt::cyan)}) {
            merge.addShotImg(frame(color), PixMergeThread::PictureDirection::Auto);
        }

        QVERIFY(waitForMergeQueue(merge, 0));
        QTest::qWait(100);
        QCOMPARE(errorSpy.count(), 0);
        merge.stopTask();
        merge.wait();
    }

    void notificationPreferenceIsForwardedToResultSink()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::white)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);

        Request request;
        request.notificationsEnabled = false;
        QVERIFY(controller.start(request));

        QCOMPARE(sink->notificationSettingCount, 1);
        QVERIFY(!sink->notificationsEnabled);
    }

    void controllerDrivesCopyThroughPorts()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::yellow)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend,
                                               overlay,
                                               nullptr,
                                               nullptr,
                                               sink);
        QSignalSpy copiedSpy(&controller, &ScreenshotSessionController::copied);

        QVERIFY(controller.start(Request()));
        overlay->press(QPoint(10, 10));
        overlay->move(QPoint(200, 150));
        overlay->release(QPoint(200, 150));
        overlay->copy();

        QCOMPARE(copiedSpy.count(), 1);
        QCOMPARE(sink->copyCount, 1);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
        QVERIFY(overlay->closed);
    }

    void controllerCopiesMagnifierColorBeforeScreenshotCopy()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::yellow)};

        auto *overlay = new FakeOverlay;
        overlay->magnifierHex = QStringLiteral("#12abef");
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend,
                                               overlay,
                                               nullptr,
                                               nullptr,
                                               sink);

        QVERIFY(controller.start(Request()));
        overlay->move(QPoint(10, 10));
        overlay->copy();

        QCOMPARE(sink->textCopyCount, 1);
        QCOMPARE(sink->copiedText, QStringLiteral("#12abef"));
        QCOMPARE(sink->copyCount, 0);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
        QVERIFY(overlay->closed);
    }

    void controllerDoesNotCompleteWhenMagnifierColorCopyFails()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::yellow)};

        auto *overlay = new FakeOverlay;
        overlay->magnifierHex = QStringLiteral("#12abef");
        auto *sink = new FakeResultSink;
        sink->textCopyResult = false;
        sink->textCopyError = QStringLiteral("剪贴板确认失败");
        ScreenshotSessionController controller(backend,
                                               overlay,
                                               nullptr,
                                               nullptr,
                                               sink);
        QSignalSpy failedSpy(&controller, &ScreenshotSessionController::failed);

        QVERIFY(controller.start(Request()));
        overlay->move(QPoint(10, 10));
        overlay->copy();

        QCOMPARE(sink->textCopyCount, 1);
        QCOMPARE(sink->copyCount, 0);
        QCOMPARE(failedSpy.count(), 1);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Failed);
        QVERIFY(overlay->closed);
    }

    void controllerMarksCursorValidOnlyAfterPointerInput()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::yellow)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend,
                                               overlay,
                                               nullptr,
                                               nullptr,
                                               sink);

        QVERIFY(controller.start(Request()));
        QVERIFY(overlay->hintsVisible);
        QVERIFY(!overlay->hintsHasGlobalCursor);

        overlay->move(QPoint(-20, -10));
        QVERIFY(overlay->hintsHasGlobalCursor);
        QCOMPARE(overlay->hintsCursor, QPoint(-20, -10));
    }

    void controllerAppliesSelectionSizeWithoutAnnotationExpansion()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red, 2.0)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *annotations = new FakeAnnotationEditor;
        annotations->bounds = QRectF(300, 100, 300, 200);
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr,
                                               sink, annotations);

        Request request;
        request.mode = Mode::FullScreen;
        QVERIFY(controller.start(request));
        overlay->requestSelectionSize(400, 300, 2.0);
        // X11 的尺寸输入只约束桌面和最小尺寸，不会因已有标注反向撑大选区。
        QCOMPARE(overlay->selection, QRect(0, 0, 200, 150));
    }

    void controllerAppliesSelectionRatioWithoutAnnotationExpansion()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *annotations = new FakeAnnotationEditor;
        annotations->bounds = QRectF(350, 100, 200, 200);
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr,
                                               sink, annotations);

        Request request;
        request.mode = Mode::FullScreen;
        QVERIFY(controller.start(request));
        overlay->requestSelectionRatio(1, 2);
        // 比例输入同样保持 X11 语义：不因标注外接矩形改变计算结果。
        QCOMPARE(overlay->selection, QRect(0, 0, 300, 600));
    }

    void controllerDrivesPinThroughPorts()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::cyan)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend,
                                               overlay,
                                               nullptr,
                                               nullptr,
                                               sink);

        QVERIFY(controller.start(Request()));
        overlay->press(QPoint(20, 30));
        overlay->move(QPoint(220, 180));
        overlay->release(QPoint(220, 180));
        overlay->pin();

        QCOMPARE(sink->pinCount, 1);
        QVERIFY(sink->lastResult.isValid());
        QCOMPARE(sink->lastResult.image.pixelColor(0, 0), QColor(Qt::cyan));
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
        QVERIFY(overlay->closed);
    }

    void controllerDrivesOcrThroughPorts()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::magenta)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);

        QVERIFY(controller.start(Request()));
        overlay->press(QPoint(20, 30));
        overlay->move(QPoint(220, 180));
        overlay->release(QPoint(220, 180));
        overlay->requestRoundedCornerRadius(12);
        overlay->ocr();

        QCOMPARE(sink->ocrCount, 1);
        QVERIFY(sink->lastResult.isValid());
        QCOMPARE(sink->lastResult.globalSelection, QRect(20, 30, 201, 151));
        // OCR uses the unrounded source image to match X11 behavior.
        QCOMPARE(sink->lastResult.image.pixelColor(0, 0), QColor(Qt::magenta));
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
        QVERIFY(overlay->closed);
    }

    void controllerDrivesTableRecognitionThroughPorts()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(100, 200, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(100, 200, 800, 600), Qt::yellow)};

        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);

        QVERIFY(controller.start(Request()));
        overlay->press(QPoint(120, 230));
        overlay->move(QPoint(320, 380));
        overlay->release(QPoint(320, 380));
        overlay->requestRoundedCornerRadius(12);
        overlay->tableRecognize();

        QCOMPARE(sink->tableRecognizeCount, 1);
        QVERIFY(sink->lastResult.isValid());
        QCOMPARE(sink->lastResult.globalSelection, QRect(120, 230, 201, 151));
        QCOMPARE(sink->lastResult.image.pixelColor(0, 0), QColor(Qt::yellow));
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
        QVERIFY(overlay->closed);
    }

    void ocrFailureFailsClosedSession()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        sink->ocrResult = false;
        sink->ocrError = QStringLiteral("OCR unavailable");
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        QSignalSpy failedSpy(&controller, &ScreenshotSessionController::failed);

        QVERIFY(controller.start(Request()));
        overlay->press(QPoint(10, 10));
        overlay->move(QPoint(100, 100));
        overlay->release(QPoint(100, 100));
        overlay->ocr();

        QCOMPARE(failedSpy.count(), 1);
        QCOMPARE(failedSpy.first().at(0).toString(), sink->ocrError);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Failed);
        QVERIFY(overlay->closed);
    }

    void tableRecognitionFailureFailsClosedSession()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        sink->tableRecognizeResult = false;
        sink->tableRecognizeError = QStringLiteral("table service unavailable");
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        QSignalSpy failedSpy(&controller, &ScreenshotSessionController::failed);

        QVERIFY(controller.start(Request()));
        overlay->press(QPoint(10, 10));
        overlay->move(QPoint(100, 100));
        overlay->release(QPoint(100, 100));
        overlay->tableRecognize();

        QCOMPARE(failedSpy.count(), 1);
        QCOMPARE(failedSpy.first().at(0).toString(), sink->tableRecognizeError);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Failed);
        QVERIFY(overlay->closed);
    }

    void controllerDrivesAiAssistantThroughPorts()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::green)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);

        QVERIFY(controller.start(Request()));
        overlay->press(QPoint(20, 30));
        overlay->move(QPoint(220, 180));
        overlay->release(QPoint(220, 180));
        overlay->aiAssistant(2);

        QCOMPARE(sink->aiAssistantCount, 1);
        QCOMPARE(sink->aiAssistantFunction, 2);
        QVERIFY(sink->lastResult.isValid());
        QCOMPARE(sink->lastResult.globalSelection, QRect(20, 30, 201, 151));
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
        QVERIFY(overlay->closed);
    }

    void aiAssistantFailureFailsClosedSession()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        sink->aiAssistantResult = false;
        sink->aiAssistantError = QStringLiteral("AI service unavailable");
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        QSignalSpy failedSpy(&controller, &ScreenshotSessionController::failed);

        QVERIFY(controller.start(Request()));
        overlay->press(QPoint(10, 10));
        overlay->move(QPoint(100, 100));
        overlay->release(QPoint(100, 100));
        overlay->aiAssistant(0);

        QCOMPARE(failedSpy.count(), 1);
        QCOMPARE(failedSpy.first().at(0).toString(), sink->aiAssistantError);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Failed);
        QVERIFY(overlay->closed);
    }

    void ocrIgnoresEmptySelection()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);

        QVERIFY(controller.start(Request()));
        overlay->ocr();

        QCOMPARE(sink->ocrCount, 0);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Selecting);
        QVERIFY(!overlay->closed);
    }

    void controllerAppliesPixelAdjustmentShortcuts()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::white)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);

        QVERIFY(controller.start(Request()));
        overlay->press(QPoint(100, 100));
        overlay->move(QPoint(400, 300));
        overlay->release(QPoint(400, 300));
        const QRect initial = overlay->selection;

        overlay->requestSelectionMove(QPoint(1, 1));
        QCOMPARE(overlay->selection, initial.translated(1, 1));

        const QRect moved = overlay->selection;
        overlay->requestSelectionResize(Qt::Key_Left, Qt::ControlModifier);
        QCOMPARE(overlay->selection.left(), moved.left() - 1);
        QCOMPARE(overlay->selection.right(), moved.right());

        const QRect enlarged = overlay->selection;
        overlay->requestSelectionResize(Qt::Key_Right,
                                        Qt::ControlModifier | Qt::ShiftModifier);
        QCOMPARE(overlay->selection.left(), enlarged.left());
        QCOMPARE(overlay->selection.right(), enlarged.right() - 1);
    }

    void selectedAnnotationOwnsResizeShortcut()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::white)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *annotations = new FakeAnnotationEditor;
        annotations->selectedContent = true;
        ScreenshotSessionController controller(backend,
                                               overlay,
                                               nullptr,
                                               nullptr,
                                               sink,
                                               annotations);

        QVERIFY(controller.start(Request()));
        overlay->press(QPoint(100, 100));
        overlay->move(QPoint(400, 300));
        overlay->release(QPoint(400, 300));
        const QRect initial = overlay->selection;

        overlay->requestSelectionResize(Qt::Key_Left, Qt::ControlModifier);
        QCOMPARE(overlay->selection, initial);
    }

    void roundedCornerRadiusIsUsedByFinalExport()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);

        QVERIFY(controller.start(Request()));
        overlay->press(QPoint(100, 100));
        overlay->move(QPoint(299, 249));
        overlay->release(QPoint(299, 249));
        overlay->requestRoundedCornerRadius(12);
        overlay->confirm();

        QVERIFY(sink->lastResult.isValid());
        QCOMPARE(sink->lastResult.image.pixelColor(0, 0).alpha(), 0);
        QCOMPARE(sink->lastResult.image.pixelColor(100, 75), QColor(Qt::red));
    }

    void roundedCornerRadiusUsesExportPixelScale()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 400, 300);
        backend->frames.outputs = {
            makeFrame(QRect(0, 0, 400, 300), Qt::red, 2.0)
        };
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);

        QVERIFY(controller.start(Request()));
        overlay->press(QPoint(100, 100));
        overlay->move(QPoint(199, 199));
        overlay->release(QPoint(199, 199));
        overlay->requestRoundedCornerRadius(10);
        overlay->confirm();

        QVERIFY(sink->lastResult.isValid());
        QCOMPARE(sink->lastResult.image.size(), QSize(200, 200));
        QCOMPARE(sink->lastResult.image.pixelColor(0, 0).alpha(), 0);
        QCOMPARE(sink->lastResult.image.pixelColor(100, 100), QColor(Qt::red));
    }

    void selectionResizeHandleHasPriorityOverAnnotation()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::white)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *annotations = new FakeAnnotationEditor;
        annotations->currentHit = {AnnotationHitKind::ResizeHandle,
                                   CursorIntent::Move,
                                   0};

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr,
                                               sink, annotations);
        Request request;
        request.mode = Mode::FullScreen;
        QVERIFY(controller.start(request));

        overlay->move(QPoint(799, 599));
        QCOMPARE(overlay->cursorIntent, CursorIntent::ResizeDiagonalForward);

        overlay->press(QPoint(799, 599));
        QCOMPARE(annotations->beginCount, 0);
        QCOMPARE(overlay->cursorIntent, CursorIntent::ResizeDiagonalForward);
    }

    void annotationHandleDrivesCursorInsideSelection()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::white)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *annotations = new FakeAnnotationEditor;
        annotations->currentHit = {AnnotationHitKind::ResizeHandle,
                                   CursorIntent::ResizeDiagonalBackward,
                                   3};

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr,
                                               sink, annotations);
        Request request;
        request.mode = Mode::FullScreen;
        QVERIFY(controller.start(request));

        overlay->move(QPoint(300, 300));
        QCOMPARE(overlay->cursorIntent, CursorIntent::ResizeDiagonalBackward);
    }

    void annotationContentUsesOpenHandUntilPressed()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::white)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *annotations = new FakeAnnotationEditor;
        annotations->beginResult = true;
        annotations->currentHit = {AnnotationHitKind::Content,
                                   CursorIntent::Move,
                                   -1};

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr,
                                               sink, annotations);
        Request request;
        request.mode = Mode::FullScreen;
        QVERIFY(controller.start(request));

        overlay->move(QPoint(200, 200));
        QCOMPARE(overlay->cursorIntent, CursorIntent::Move);

        overlay->press(QPoint(200, 200));
        QCOMPARE(overlay->cursorIntent, CursorIntent::Grabbing);

        overlay->move(QPoint(300, 300));
        QCOMPARE(overlay->cursorIntent, CursorIntent::Grabbing);

        overlay->release(QPoint(300, 300));
        QCOMPARE(overlay->cursorIntent, CursorIntent::Move);
    }

    void annotationActionAndCursorStayLockedUntilRelease()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::white)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *annotations = new FakeAnnotationEditor;
        annotations->beginResult = true;
        annotations->currentHit = {AnnotationHitKind::Content,
                                   CursorIntent::Move,
                                   -1};

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr,
                                               sink, annotations);
        Request request;
        request.mode = Mode::FullScreen;
        QVERIFY(controller.start(request));

        overlay->press(QPoint(200, 200));
        QVERIFY(annotations->interacting);
        QCOMPARE(overlay->cursorIntent, CursorIntent::Grabbing);

        annotations->currentHit = {AnnotationHitKind::ResizeHandle,
                                   CursorIntent::ResizeDiagonalForward,
                                   3};
        overlay->move(QPoint(600, 500));
        QCOMPARE(annotations->updateCount, 1);
        QCOMPARE(annotations->updatePositions.last(), QPoint(600, 500));
        QCOMPARE(overlay->cursorIntent, CursorIntent::Grabbing);

        overlay->release(QPoint(600, 500));
        QVERIFY(!annotations->interacting);
        QCOMPARE(annotations->endCount, 1);
    }

    void selectionMoveCursorChangesWhilePressed()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::white)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        Request request;
        request.mode = Mode::FullScreen;
        QVERIFY(controller.start(request));

        overlay->move(QPoint(400, 300));
        QCOMPARE(overlay->cursorIntent, CursorIntent::Move);

        overlay->press(QPoint(400, 300));
        QCOMPARE(overlay->cursorIntent, CursorIntent::Grabbing);

        overlay->release(QPoint(400, 300));
        QCOMPARE(overlay->cursorIntent, CursorIntent::Move);
    }

    void annotationBoundsKeepReadySelectionFixed()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::white)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *annotations = new FakeAnnotationEditor;
        annotations->beginResult = true;
        annotations->currentHit = {AnnotationHitKind::Content,
                                   CursorIntent::Move,
                                   -1};

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr,
                                               sink, annotations);
        Request request;
        request.initialRegion = QRect(100, 100, 200, 200);
        QVERIFY(controller.start(request));
        overlay->press(QPoint(150, 150));
        overlay->release(QPoint(150, 150));
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Editing);
        const QRect original = overlay->selection;

        annotations->setBounds(QRectF(80, 90, 260, 240));
        QCOMPARE(overlay->selection, original);
    }

    void annotationDrawingCanExtendBeyondSelection()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::white)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *annotations = new FakeAnnotationEditor;
        annotations->beginResult = true;
        annotations->currentHit = {AnnotationHitKind::CreateContent,
                                   CursorIntent::Cross,
                                   -1};

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr,
                                               sink, annotations);
        Request request;
        request.initialRegion = QRect(100, 100, 200, 200);
        QVERIFY(controller.start(request));
        const QRect original = overlay->selection;
        QCOMPARE(annotations->interactionBounds, original);

        overlay->press(QPoint(150, 150));
        overlay->move(QPoint(420, 390));
        QCOMPARE(annotations->updateCount, 1);
        QCOMPARE(annotations->updatePositions.last(), QPoint(420, 390));
        QCOMPARE(overlay->selection, original);
    }

    void initialSelectionIsSyncedAfterOverlayCreation()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::white)};
        auto *overlay = new FakeOverlay;
        overlay->clearSelectionOnCreate = true;
        auto *sink = new FakeResultSink;

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        Request request;
        request.mode = Mode::FullScreen;
        QVERIFY(controller.start(request));

        QCOMPARE(overlay->selection, backend->frames.virtualDesktopRect);
        QVERIFY(overlay->finalized);
    }

    void crossOutputCompositorSnapIsResolvedOnDominantOutput()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 3840, 1080);
        backend->frames.outputs = {
            makeFrame(QRect(0, 0, 1920, 1080), Qt::red),
            makeFrame(QRect(1920, 0, 1920, 1080), Qt::blue)
        };
        auto *overlay = new FakeOverlay;
        auto *snapHints = new FakeSnapHints;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend,
                                               overlay,
                                               snapHints,
                                               nullptr,
                                               sink);

        Request request;
        request.enableWindowSnap = false;
        QVERIFY(controller.start(request));
        snapHints->setRegion(QRect(1904, -10, 1952, 1085));

        QCOMPARE(overlay->snapHint, QRect(1920, 0, 1920, 1080));
    }

    void overlayEditorPublishedDuringCreationIsBoundBeforeInput()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::white)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        auto *annotations = new FakeAnnotationEditor;
        annotations->beginResult = true;
        annotations->currentHit = {AnnotationHitKind::Content,
                                   CursorIntent::Move,
                                   -1};
        overlay->pendingEditor = annotations;

        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);

        QVERIFY(controller.start(Request()));
        overlay->press(QPoint(10, 10));
        overlay->move(QPoint(300, 300));
        overlay->release(QPoint(300, 300));
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Editing);

        overlay->press(QPoint(100, 100));

        QCOMPARE(annotations->beginCount, 1);
        QCOMPARE(overlay->cursorIntent, CursorIntent::Grabbing);
    }

    void fullScreenRequestUsesVirtualDesktop()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(-1920, 0, 3840, 1080);
        backend->frames.outputs = {
            makeFrame(QRect(-1920, 0, 1920, 1080), Qt::red),
            makeFrame(QRect(0, 0, 1920, 1080), Qt::blue)
        };
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);

        Request request;
        request.mode = Mode::FullScreen;
        QVERIFY(controller.start(request));
        QCOMPARE(overlay->selection, backend->frames.virtualDesktopRect);
        QVERIFY(overlay->finalized);

        overlay->confirm();
        QCOMPARE(sink->saveCount, 1);
        QCOMPARE(sink->lastResult.image.size(), QSize(3840, 1080));
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
    }

    void fullScreenRequestAutoSavesWithoutConfirmation()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::red)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);

        Request request;
        request.mode = Mode::FullScreen;
        request.outputPath = QStringLiteral("/tmp/fullscreen.png");

        QVERIFY(controller.start(request));
        QTRY_COMPARE_WITH_TIMEOUT(sink->saveCount, 1, 2000);
        QCOMPARE(controller.state(), ScreenshotSessionController::State::Completed);
        QCOMPARE(sink->requestedPath, QStringLiteral("/tmp/fullscreen.png"));
        QVERIFY(sink->lastResult.isValid());
    }

    void overlayMappingFailureFailsSession()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 1920, 1080);
        backend->frames.outputs = {
            makeFrame(QRect(0, 0, 1920, 1080), Qt::white)
        };
        auto *overlay = new FakeOverlay;
        overlay->readyOnShow = false;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        QSignalSpy failedSpy(&controller, &ScreenshotSessionController::failed);

        QVERIFY(controller.start(Request()));
        QCOMPARE(controller.state(), ScreenshotSessionController::State::CreatingMasks);

        overlay->failInput(QStringLiteral("覆盖窗口映射失败"));

        QCOMPARE(controller.state(), ScreenshotSessionController::State::Failed);
        QCOMPARE(failedSpy.size(), 1);
        QVERIFY(overlay->closed);
        QCOMPARE(sink->saveCount, 0);
        QCOMPARE(sink->copyCount, 0);
    }

    void overlayPreparationRunsBeforeShow()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 1920, 1080);
        backend->frames.outputs = {
            makeFrame(QRect(0, 0, 1920, 1080), Qt::white)
        };
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);

        QVERIFY(controller.start(Request()));

        QCOMPARE(controller.state(), ScreenshotSessionController::State::Selecting);
        QCOMPARE(overlay->prepareCount, 1);
        QCOMPARE(overlay->lifecycle,
                 QStringList({QStringLiteral("prepare"), QStringLiteral("show")}));
    }

    void overlayPreparationFailureAbortsBeforeShow()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 1920, 1080);
        backend->frames.outputs = {
            makeFrame(QRect(0, 0, 1920, 1080), Qt::white)
        };
        auto *overlay = new FakeOverlay;
        overlay->prepareResult = false;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        QSignalSpy failedSpy(&controller, &ScreenshotSessionController::failed);

        QVERIFY(controller.start(Request()));

        QCOMPARE(controller.state(), ScreenshotSessionController::State::Failed);
        QCOMPARE(failedSpy.size(), 1);
        QVERIFY(!overlay->shown);
        QVERIFY(overlay->closed);
        QCOMPARE(overlay->lifecycle, QStringList({QStringLiteral("prepare")}));
    }

    void snapMaskPreparationFailureAbortsBeforeShow()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 1920, 1080);
        backend->frames.outputs = {
            makeFrame(QRect(0, 0, 1920, 1080), Qt::white)
        };
        auto *overlay = new FakeOverlay;
        auto *snapHints = new FakeSnapHints;
        auto *tagger = new FakeWindowTagger;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend,
                                               overlay,
                                               snapHints,
                                               tagger,
                                               sink);
        QSignalSpy failedSpy(&controller, &ScreenshotSessionController::failed);

        QVERIFY(controller.start(Request()));

        QCOMPARE(controller.state(), ScreenshotSessionController::State::Failed);
        QCOMPARE(failedSpy.size(), 1);
        QVERIFY(!overlay->shown);
        QVERIFY(overlay->closed);
        QCOMPARE(overlay->lifecycle, QStringList({QStringLiteral("prepare")}));
    }

    void cancelClosesOverlayWithoutConsumingResult()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::white)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);

        QVERIFY(controller.start(Request()));
        overlay->cancel();

        QCOMPARE(controller.state(), ScreenshotSessionController::State::Cancelled);
        QCOMPARE(sink->saveCount, 0);
        QCOMPARE(sink->copyCount, 0);
        QVERIFY(overlay->closed);
    }

    void saveCancellationEndsSessionWithoutFailure()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::white)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        sink->saveResult = false;
        sink->saveCancelled = true;
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        QSignalSpy failedSpy(&controller, &ScreenshotSessionController::failed);

        QVERIFY(controller.start(Request()));
        overlay->press(QPoint(10, 10));
        overlay->move(QPoint(200, 150));
        overlay->release(QPoint(200, 150));
        overlay->confirm();

        QCOMPARE(controller.state(), ScreenshotSessionController::State::Cancelled);
        QCOMPARE(failedSpy.size(), 0);
        QVERIFY(overlay->closed);
    }

    void saveFailureEndsSessionWithFailure()
    {
        FakeBackend *backend = new FakeBackend;
        backend->frames.virtualDesktopRect = QRect(0, 0, 800, 600);
        backend->frames.outputs = {makeFrame(QRect(0, 0, 800, 600), Qt::white)};
        auto *overlay = new FakeOverlay;
        auto *sink = new FakeResultSink;
        sink->saveResult = false;
        sink->saveError = QStringLiteral("保存失败");
        ScreenshotSessionController controller(backend, overlay, nullptr, nullptr, sink);
        QSignalSpy failedSpy(&controller, &ScreenshotSessionController::failed);

        QVERIFY(controller.start(Request()));
        overlay->press(QPoint(10, 10));
        overlay->move(QPoint(200, 150));
        overlay->release(QPoint(200, 150));
        overlay->confirm();

        QCOMPARE(controller.state(), ScreenshotSessionController::State::Failed);
        QCOMPARE(failedSpy.size(), 1);
        QCOMPARE(failedSpy.takeFirst().at(0).toString(), QStringLiteral("保存失败"));
        QVERIFY(overlay->closed);
    }

    void recordingTransactionDiscardPreservesExistingDestination()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString finalPath = directory.filePath(QStringLiteral("recording.mp4"));
        QFile finalFile(finalPath);
        QVERIFY(finalFile.open(QIODevice::WriteOnly));
        QCOMPARE(finalFile.write("previous recording"), qint64(18));
        finalFile.close();

        RecordingFileTransaction transaction(finalPath);
        const QString capturePath = transaction.createTemporaryPath(QStringLiteral("mp4"));
        const QString convertedPath = transaction.createTemporaryPath(QStringLiteral("gif"));
        QVERIFY(!capturePath.isEmpty());
        QVERIFY(!convertedPath.isEmpty());

        QFile captureFile(capturePath);
        QVERIFY(captureFile.open(QIODevice::WriteOnly));
        QCOMPARE(captureFile.write("partial capture"), qint64(15));
        captureFile.close();

        QFile convertedFile(convertedPath);
        QVERIFY(convertedFile.open(QIODevice::WriteOnly));
        QCOMPARE(convertedFile.write("partial transcode"), qint64(17));
        convertedFile.close();

        QVERIFY(transaction.discard());
        QVERIFY(QFileInfo::exists(finalPath));
        QVERIFY(!QFileInfo::exists(capturePath));
        QVERIFY(!QFileInfo::exists(convertedPath));

        QVERIFY(finalFile.open(QIODevice::ReadOnly));
        QCOMPARE(finalFile.readAll(), QByteArrayLiteral("previous recording"));
    }

    void recordingTransactionPublishesOnlyAfterSuccess()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString finalPath = directory.filePath(QStringLiteral("recording.mkv"));
        QFile finalFile(finalPath);
        QVERIFY(finalFile.open(QIODevice::WriteOnly));
        QCOMPARE(finalFile.write("previous recording"), qint64(18));
        finalFile.close();

        RecordingFileTransaction transaction(finalPath);
        const QString capturePath = transaction.createTemporaryPath(QStringLiteral("mkv"));
        QVERIFY(!capturePath.isEmpty());

        QFile captureFile(capturePath);
        QVERIFY(captureFile.open(QIODevice::WriteOnly));
        QCOMPARE(captureFile.write("completed recording"), qint64(19));
        captureFile.close();

        QString error;
        QVERIFY2(transaction.publish(capturePath, &error), qPrintable(error));
        QVERIFY(!QFileInfo::exists(capturePath));
        QVERIFY(finalFile.open(QIODevice::ReadOnly));
        QCOMPARE(finalFile.readAll(), QByteArrayLiteral("completed recording"));
    }

    void recordingAudioArgumentsKeepNoAudioPathUnchanged()
    {
        AudioCaptureOptions audio;
        const QStringList args = FramePumpWorker::buildFfmpegArguments(
                1920, 1080, 24, QString(), audio, QStringLiteral("/tmp/out.mp4"));

        QVERIFY(args.contains("-map"));
        QCOMPARE(args.at(args.lastIndexOf("-map") + 1), QStringLiteral("0:v:0"));
        QVERIFY(!args.contains("pulse"));
        QVERIFY(!args.contains("-c:a"));
        QVERIFY(!args.contains("-shortest"));
        QVERIFY(!args.contains("-vsync"));
        QCOMPARE(args.at(args.indexOf("-fps_mode") + 1), QStringLiteral("cfr"));
        QCOMPARE(args.at(args.lastIndexOf("-r") + 1), QStringLiteral("24"));
    }

    void recordingAudioArgumentsMapMicrophoneInput()
    {
        AudioCaptureOptions audio;
        audio.type = AudioCaptureOptions::Microphone;
        const QStringList args = FramePumpWorker::buildFfmpegArguments(
                1920, 1080, 30, QStringLiteral("crop=1280:720:0:0"),
                audio, QStringLiteral("/tmp/out.mp4"));

        QVERIFY(args.contains("pulse"));
        QVERIFY(args.contains(QStringLiteral("default")));
        QVERIFY(args.indexOf("pulse") < args.indexOf("rawvideo"));
        QVERIFY(args.contains("-vf"));
        QVERIFY(args.at(args.indexOf("-vf") + 1).startsWith(QStringLiteral("crop=1280:720:0:0,")));
        QCOMPARE(args.at(args.indexOf("-map") + 1), QStringLiteral("1:v:0"));
        QCOMPARE(args.at(args.lastIndexOf("-map") + 1), QStringLiteral("0:a:0"));
        QVERIFY(args.contains("-shortest"));
        QVERIFY(args.contains("aac"));
        QCOMPARE(args.at(args.lastIndexOf("-r") + 1), QStringLiteral("30"));
    }

    void recordingAudioArgumentsMapSystemInput()
    {
        AudioCaptureOptions audio;
        audio.type = AudioCaptureOptions::SystemAudio;
        audio.systemDevice = QStringLiteral("87");
        const QStringList args = FramePumpWorker::buildFfmpegArguments(
                2560, 1440, 24, QString(), audio, QStringLiteral("/tmp/out.mp4"));

        QVERIFY(args.contains(QStringLiteral("87")));
        QVERIFY(args.indexOf("pulse") < args.indexOf("rawvideo"));
        QCOMPARE(args.at(args.indexOf("-map") + 1), QStringLiteral("1:v:0"));
        QCOMPARE(args.at(args.lastIndexOf("-map") + 1), QStringLiteral("0:a:0"));
        QVERIFY(args.contains("-shortest"));
    }

    void recordingAudioArgumentsMixMicrophoneAndSystemInputs()
    {
        AudioCaptureOptions audio;
        audio.type = AudioCaptureOptions::MicrophoneAndSystemAudio;
        audio.systemDevice = QStringLiteral("87");
        const QStringList args = FramePumpWorker::buildFfmpegArguments(
                2560, 1440, 24, QString(), audio, QStringLiteral("/tmp/out.mp4"));

        const int filterIndex = args.indexOf("-filter_complex");
        QVERIFY(filterIndex >= 0);
        const QString filter = args.at(filterIndex + 1);
        QVERIFY(filter.contains(QStringLiteral("amerge=inputs=2")));
        QVERIFY(filter.contains(QStringLiteral("pan=stereo")));
        QVERIFY(args.indexOf("pulse") < args.indexOf("rawvideo"));
        QVERIFY(filter.startsWith(QStringLiteral("[0:a][1:a]")));
        QVERIFY(!args.contains("[vout]"));
        QVERIFY(!args.contains("[aout]"));
        QVERIFY(args.contains("-vf"));
        QVERIFY(args.contains("-shortest"));
    }

    void recordingAudioOptionsFallbackSemantics()
    {
        AudioCaptureOptions audio;
        QVERIFY(!audio.hasAudio());

        audio.type = AudioCaptureOptions::Microphone;
        QVERIFY(audio.hasMicrophone());
        QVERIFY(!audio.hasSystemAudio());

        audio.type = AudioCaptureOptions::SystemAudio;
        QVERIFY(!audio.hasMicrophone());
        QVERIFY(audio.hasSystemAudio());

        audio.type = AudioCaptureOptions::MicrophoneAndSystemAudio;
        QVERIFY(audio.hasAudio());
    }

    void recordingArgumentsUseWallclockPtsOnlyForRawVideoInput()
    {
        AudioCaptureOptions audio;
        audio.type = AudioCaptureOptions::MicrophoneAndSystemAudio;
        audio.systemDevice = QStringLiteral("87");
        const QStringList args = FramePumpWorker::buildFfmpegArguments(
                2560, 1440, 24, QString(), audio, QStringLiteral("/tmp/out.mp4"));

        const int wallclockIndex = args.indexOf(QStringLiteral("-use_wallclock_as_timestamps"));
        QVERIFY(wallclockIndex >= 0);
        QCOMPARE(args.at(wallclockIndex + 1), QStringLiteral("1"));

        const int rawVideoIndex = args.indexOf(QStringLiteral("rawvideo"));
        QVERIFY(rawVideoIndex > wallclockIndex);
        QVERIFY(wallclockIndex > args.lastIndexOf(QStringLiteral("pulse")));
        QVERIFY(wallclockIndex < args.lastIndexOf(QStringLiteral("-i")));

        const int fpsModeIndex = args.indexOf(QStringLiteral("-fps_mode"));
        QVERIFY(fpsModeIndex >= 0);
        QCOMPARE(args.at(fpsModeIndex + 1), QStringLiteral("cfr"));
        QCOMPARE(args.at(args.lastIndexOf("-r") + 1), QStringLiteral("24"));

        QVERIFY(!args.contains(QStringLiteral("-vsync")));
        QVERIFY(!args.join(' ').contains(QStringLiteral("itsscale")));
        QVERIFY(!args.join(' ').contains(QStringLiteral("passthrough")));
    }
};

QTEST_MAIN(ScreenshotSessionTest)

#include "main.moc"
