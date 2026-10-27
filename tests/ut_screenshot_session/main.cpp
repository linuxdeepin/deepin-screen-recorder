// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenshot/core/selectionmodel.h"
#include "screenshot/core/selectionoverlaygeometry.h"
#include "screenshot/export/screenshotexportservice.h"
#include "screenshot/ports/iannotationeditor.h"
#include "screenshot/ports/iscreenshotbackend.h"
#include "screenshot/ports/iscreenshotoverlayhost.h"
#include "screenshot/ports/iscreenshotresultsink.h"
#include "screenshot/ports/isnaphintsource.h"
#include "screenshot/ports/iwindowtagger.h"
#include "screenshot/session/annotationcoordinator.h"
#include "screenshot/session/screenshotsessioncontroller.h"
#include "utils/shapesutils.h"

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QSignalSpy>
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
    QList<QPoint> updatePositions;
    AnnotationTool currentTool = AnnotationTool::Pointer;

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

    void setSelectionFinalized(bool value) override
    {
        finalized = value;
    }

    void setCursorIntent(CursorIntent intent) override
    {
        cursorIntent = intent;
    }

    void close() override
    {
        closed = true;
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

    void cancel()
    {
        emit cancelRequested();
    }

    void failInput(const QString &reason)
    {
        emit inputFailed(reason);
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
    int pinCount = 0;
    QString requestedPath;
    Result lastResult;
    bool saveResult = true;
    bool saveCancelled = false;
    QString saveError;

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

    bool pin(const Result &result, QString *) override
    {
        ++pinCount;
        lastResult = result;
        return true;
    }
};

} // namespace

class ScreenshotSessionTest : public QObject
{
    Q_OBJECT
private slots:
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

    void selectionShrinkCannotCropAnnotations()
    {
        SelectionModel model;
        model.setDesktopBounds(QRect(0, 0, 1000, 800));
        model.setGeometry(QRect(100, 100, 600, 400));
        model.setAnnotationBounds(QRectF(200, 200, 200, 100));

        QCOMPARE(model.beginInteraction(QPoint(699, 300)), SelectionAction::ResizeRight);
        model.updateInteraction(QPoint(250, 300));
        model.endInteraction();

        const QRect result = model.geometry();
        QVERIFY(result.left() <= 200);
        QVERIFY(result.right() >= 399);
        QVERIFY(result.top() <= 200);
        QVERIFY(result.bottom() >= 299);
    }

    void annotationBoundsUpdateExpandsReadySelection()
    {
        SelectionModel model;
        model.setDesktopBounds(QRect(0, 0, 1000, 800));
        model.setGeometry(QRect(100, 100, 300, 200));

        FakeAnnotationEditor editor;
        AnnotationCoordinator coordinator(&model, &editor);
        editor.setBounds(QRectF(750, 150, 100, 100));

        QVERIFY(model.geometry().contains(QPoint(750, 150)));
        QVERIFY(model.geometry().contains(QPoint(849, 249)));
    }

    void annotationBoundsAcrossOutputsExpandReadySelection()
    {
        SelectionModel model;
        model.setDesktopBounds(QRect(0, 0, 3840, 1080));
        model.setGeometry(QRect(100, 100, 300, 200));

        FakeAnnotationEditor editor;
        AnnotationCoordinator coordinator(&model, &editor);
        editor.setBounds(QRectF(100, 100, 2100, 600));

        const QRect selection = model.geometry();
        QVERIFY(selection.left() <= 100);
        QVERIFY(selection.right() >= 2199);
        QVERIFY(selection.top() <= 100);
        QVERIFY(selection.bottom() >= 699);
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
        QVERIFY(overlay->closed);
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

    void annotationBoundsExpandReadySelection()
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

        annotations->setBounds(QRectF(80, 90, 260, 240));
        QVERIFY(overlay->selection.contains(QRect(80, 90, 260, 240)));
    }

    void annotationInteractionExpandsSelectionImmediately()
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
        annotations->setBounds(QRectF(60, 70, 300, 260));
        QVERIFY(overlay->selection.contains(QRect(60, 70, 300, 260)));

        overlay->move(QPoint(420, 390));
        QCOMPARE(annotations->updateCount, 1);
        QVERIFY(overlay->selection.contains(QRect(60, 70, 300, 260)));
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
};

QTEST_GUILESS_MAIN(ScreenshotSessionTest)

#include "main.moc"
