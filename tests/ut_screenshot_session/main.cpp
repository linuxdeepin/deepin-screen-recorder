// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenshot/core/selectionmodel.h"
#include "screenshot/export/screenshotexportservice.h"
#include "screenshot/ports/iannotationeditor.h"
#include "screenshot/ports/iscreenshotbackend.h"
#include "screenshot/ports/iscreenshotoverlayhost.h"
#include "screenshot/ports/iscreenshotresultsink.h"
#include "screenshot/ports/isnaphintsource.h"
#include "screenshot/ports/iwindowtagger.h"
#include "screenshot/session/annotationcoordinator.h"
#include "screenshot/session/screenshotsessioncontroller.h"

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

    QRectF contentBoundsGlobal() const override
    {
        return bounds;
    }

    bool isInteracting() const override
    {
        return false;
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
    bool finalized = false;
    bool shown = false;
    bool closed = false;

    bool create(const QList<OutputDescriptor> &outputs) override
    {
        createdOutputs = outputs;
        return !outputs.isEmpty();
    }

    QList<QWindow *> maskWindows() const override
    {
        return {};
    }

    void setFrozenFrames(const FrozenFrameSet &) override
    {
    }

    void show() override
    {
        shown = true;
        emit inputReady();
    }

    void setSelection(const QRect &globalSelection) override
    {
        selection = globalSelection;
    }

    void setSnapHint(const QRect &) override
    {
    }

    void setSelectionFinalized(bool value) override
    {
        finalized = value;
    }

    void setCursorIntent(CursorIntent) override
    {
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

    void cancel()
    {
        emit cancelRequested();
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
    QString requestedPath;
    Result lastResult;

    bool save(const Result &result,
              const QString &path,
              QString *savedPath,
              QString *) override
    {
        ++saveCount;
        requestedPath = path;
        lastResult = result;
        if (savedPath)
            *savedPath = path;
        return true;
    }

    bool copy(const Result &result, QString *) override
    {
        ++copyCount;
        lastResult = result;
        return true;
    }
};

} // namespace

class ScreenshotSessionTest : public QObject
{
    Q_OBJECT
private slots:
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
};

QTEST_GUILESS_MAIN(ScreenshotSessionTest)

#include "main.moc"
