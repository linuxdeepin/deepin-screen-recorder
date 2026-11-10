// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenshotsessionfactory.h"

#include "screenshotsessioncontroller.h"
#include "platformdetection.h"
#include "../export/localscreenshotresultsink.h"
#include "../platform/treeland/treelandmaskoverlayhost.h"
#include "../platform/treeland/treelandscreenshotbackend.h"
#include "../platform/treeland/treelandsnaphintsource.h"
#include "../platform/treeland/treelandsnapproxy.h"
#include "../platform/treeland/treelandwindowtagger.h"
#include "../platform/treeland/treelandrecordingsession.h"
#include "../platform/treeland/treelandscrollcapture.h"
#include "../merge/scrollmergecoordinator.h"

#include <QGuiApplication>
#include <QProcessEnvironment>

namespace Dsr::Screenshot {

namespace {

} // namespace

bool ScreenshotSessionFactory::isCurrentPlatformSupported()
{
    if (!isTreeLandEnvironment())
        return false;

    return QGuiApplication::platformName().startsWith(QStringLiteral("wayland"),
                                                      Qt::CaseInsensitive);
}

ScreenshotSessionController *ScreenshotSessionFactory::createForCurrentPlatform(QObject *parent,
                                                                                QString *error)
{
    if (!isCurrentPlatformSupported()) {
        if (error)
            *error = QStringLiteral("当前平台没有可用的截图会话后端");
        return nullptr;
    }

    auto *snapSession = new TreelandSnapSession();
    auto *backend = new TreelandScreenshotBackend(snapSession);
    auto *snapHints = new TreelandSnapHintSource(snapSession);
    auto *windowTagger = new TreelandWindowTagger(snapSession);
    auto *overlay = new TreelandMaskOverlayHost(windowTagger);
    auto *resultSink = new LocalScreenshotResultSink();
    auto *annotationEditor = overlay->annotationEditor();

    auto *controller = new ScreenshotSessionController(backend,
                                                       overlay,
                                                       snapHints,
                                                       windowTagger,
                                                       resultSink,
                                                       annotationEditor,
                                                       parent);
    controller->setRecordingSession(new TreelandRecordingSession);
    auto *scrollCapture = new TreelandScrollCapture;
    scrollCapture->setSnapSession(snapSession);
    controller->setScrollCapture(scrollCapture);
    controller->setScrollMerger(new ScrollMergeCoordinator);
    snapSession->setParent(controller);
    return controller;
}

} // namespace Dsr::Screenshot
