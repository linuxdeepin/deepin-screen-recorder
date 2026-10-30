// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "screenshot/core/screenshottypes.h"
#include "screenshot/core/selectionmodel.h"
#include "screenshot/ports/iannotationeditor.h"
#include "screenshot/ports/iscreenshotbackend.h"
#include "screenshot/ports/iscreenshotoverlayhost.h"
#include "screenshot/ports/iscreenshotresultsink.h"
#include "screenshot/ports/isnaphintsource.h"
#include "screenshot/ports/iwindowtagger.h"

#include <QColor>
#include <QImage>
#include <QPainter>

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

    QRectF contentBoundsGlobal() const override { return bounds; }
    bool isInteracting() const override { return interacting; }
    void setTool(AnnotationTool tool) override { currentTool = tool; }
    AnnotationTool tool() const override { return currentTool; }
    AnnotationHit hitTest(const QPoint &) const override { ++hitTestCount; return currentHit; }
    bool beginInteraction(const QPoint &) override { ++beginCount; interacting = beginResult; return beginResult; }
    void updateInteraction(const QPoint &globalPos) override { ++updateCount; updatePositions.append(globalPos); }
    void endInteraction() override { ++endCount; interacting = false; }
    void cancelInteraction() override { ++cancelCount; interacting = false; }
    void render(QPainter &painter, const QRect &, qreal) const override
    {
        renderCalled = true;
        if (renderColor.alpha() > 0)
            painter.fillRect(QRectF(0, 0, 10000, 10000), renderColor);
    }
    void setBounds(const QRectF &newBounds) { bounds = newBounds; emit contentBoundsChanged(bounds); }
};

class FakeBackend : public IScreenshotBackend
{
public:
    FrozenFrameSet frames;
    int prepareCount = 0;
    int cancelCount = 0;

    BackendCapabilities capabilities() const override { return {}; }
    QList<OutputDescriptor> outputs() const override
    {
        QList<OutputDescriptor> result;
        for (const OutputFrame &frame : frames.outputs)
            result.append(frame.output);
        return result;
    }
    void prepare(const Request &) override { ++prepareCount; emit prepared(frames); }
    void cancel() override { ++cancelCount; }
};

class FakeOverlay : public IScreenshotOverlayHost
{
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
    IAnnotationEditor *annotationEditor() const override { return providedEditor; }
    QList<QWindow *> maskWindows() const override { return {}; }
    void setFrozenFrames(const FrozenFrameSet &) override {}
    bool prepare() override { ++prepareCount; lifecycle.append(QStringLiteral("prepare")); return prepareResult; }
    void show() override { shown = true; lifecycle.append(QStringLiteral("show")); if (readyOnShow) emit inputReady(); }
    void setSelection(const QRect &globalSelection) override { selection = globalSelection; }
    void setSnapHint(const QRect &globalRegion) override { snapHint = globalRegion; }
    void setSelectionFinalized(bool f) override { finalized = f; }
    void setCursorIntent(CursorIntent intent) override { cursorIntent = intent; }
    void close() override { closed = true; lifecycle.append(QStringLiteral("close")); }

    void press(const QPoint &p) { emit pointerPressed(p); }
    void move(const QPoint &p) { emit pointerMoved(p); }
    void release(const QPoint &p) { emit pointerReleased(p); }
    void confirm() { emit confirmRequested(); }
    void copy() { emit copyRequested(); }
    void pin() { emit pinRequested(); }
    void cancel() { emit cancelRequested(); }
    void fail(const QString &reason) { emit inputFailed(reason); }
};

class FakeResultSink : public IScreenshotResultSink
{
public:
    int saveCount = 0;
    int copyCount = 0;
    int pinCount = 0;
    bool saveResult = true;
    bool copyResult = true;
    bool pinResult = true;
    bool saveCancelled = false;
    QString saveError;
    QString copyError;
    QString pinError;
    QString lastSavedPath;
    Result lastSaveResult;
    Result lastCopyResult;
    Result lastPinResult;

    bool save(const Result &result, const QString &, QString *savedPath,
              QString *error, bool *cancelled) override
    {
        ++saveCount;
        lastSaveResult = result;
        if (saveCancelled) {
            if (cancelled) *cancelled = true;
            return false;
        }
        if (!saveResult) {
            if (error) *error = saveError;
            return false;
        }
        if (savedPath) *savedPath = QStringLiteral("/tmp/test-save.png");
        lastSavedPath = QStringLiteral("/tmp/test-save.png");
        return true;
    }
    bool copy(const Result &result, QString *error) override
    {
        ++copyCount;
        lastCopyResult = result;
        if (!copyResult) { if (error) *error = copyError; return false; }
        return true;
    }
    bool pin(const Result &result, QString *error) override
    {
        ++pinCount;
        lastPinResult = result;
        if (!pinResult) { if (error) *error = pinError; return false; }
        return true;
    }
};

class FakeSnapHints : public ISnapHintSource
{
public:
    int startCount = 0;
    int stopCount = 0;
    bool startResult = true;
    QString startError;
    QRect region;
    bool start(QString *error) override { ++startCount; if (!startResult && error) *error = startError; return startResult; }
    void stop() override { ++stopCount; }
    QRect currentRegion() const override { return region; }
    void emitRegion(const QRect &r) { region = r; emit regionChanged(r); }
};

class FakeWindowTagger : public IWindowTagger
{
public:
    int tagCount = 0;
    bool tagResult = true;
    QString tagError;
    bool tagMaskWindow(QWindow *, QString *error) override { ++tagCount; if (!tagResult && error) *error = tagError; return tagResult; }
};

} // anonymous namespace
