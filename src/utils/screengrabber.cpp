// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screengrabber.h"

#include "../utils.h"
#include "../utils/log.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QFile>
#include <QPixmap>
#include <QScreen>
#include <QGuiApplication>
#include <QPainter>

ScreenGrabber::ScreenGrabber(QObject *parent) : QObject(parent)
{
    qCDebug(dsrApp) << "ScreenGrabber initialized.";
}

/**
 * @brief Capture a specified rectangular area of the entire desktop
 * @param ok Output parameter indicating whether the screenshot was successful
 * @param rect The rectangular area to capture (in virtual desktop coordinates)
 * @param devicePixelRatio Device pixel ratio for high-DPI displays
 * @return The captured desktop image, or an empty image if failed
 * 
 * This function serves as the main entry point for screenshot functionality.
 * It automatically selects the appropriate screenshot method based on the current
 * display server type (Wayland/X11) and handles complex cross-screen scenarios.
 */
QPixmap ScreenGrabber::grabEntireDesktop(bool &ok, const QRect &rect, const qreal devicePixelRatio)
{
    qCDebug(dsrApp) << "Grabbing entire desktop for rect:" << rect << ", devicePixelRatio:" << devicePixelRatio;
    ok = true;
    if (Utils::isWaylandMode) {
        qCDebug(dsrApp) << "Wayland mode detected, using Wayland screenshot.";
        return grabWaylandScreenshot(ok, rect, devicePixelRatio);
    }
    qCDebug(dsrApp) << "X11 mode detected, using X11 screenshot.";
    return grabX11Screenshot(ok, rect);
}

/**
 * @brief Capture desktop using KWin D-Bus interface in Wayland mode
 * @param ok Output parameter indicating screenshot success
 * @param rect The rectangular area to capture
 * @param devicePixelRatio Device pixel ratio
 * @return The captured desktop image
 * 
 * Wayland's security model prevents applications from directly accessing screen content.
 * Screenshots must be obtained through the compositor's (KWin) D-Bus interface.
 */
QPixmap ScreenGrabber::grabWaylandScreenshot(bool &ok, const QRect &rect, const qreal devicePixelRatio)
{
    qCDebug(dsrApp) << "Grabbing Wayland screenshot for rect:" << rect;
    const QRect recordRect(
        static_cast<int>(rect.x() * devicePixelRatio),
        static_cast<int>(rect.y() * devicePixelRatio),
        static_cast<int>(rect.width() * devicePixelRatio),
        static_cast<int>(rect.height() * devicePixelRatio)
    );
    
    QDBusInterface kwinInterface(
        QStringLiteral("org.kde.KWin"),
        QStringLiteral("/Screenshot"),
        QStringLiteral("org.kde.kwin.Screenshot")
    );
    
    QDBusReply<QString> reply = kwinInterface.call(QStringLiteral("screenshotFullscreen"));
    QPixmap fullScreenshot(reply.value());
    
    if (!fullScreenshot.isNull()) {
        qCDebug(dsrApp) << "Full screenshot successful, removing temp file and copying region.";
        QFile::remove(reply.value());
        return fullScreenshot.copy(recordRect);
    }
    
    qCWarning(dsrApp) << "Full screenshot failed, returning empty pixmap.";
    ok = false;
    return {};
}

/**
 * @brief Intelligently handle multi-screen screenshots in X11 mode
 * @param ok Output parameter indicating screenshot success
 * @param rect The rectangular area to capture
 * @return The captured desktop image
 * 
 * In X11 mode, screen content can be accessed directly. This function intelligently
 * detects how many screens are involved in the screenshot area and uses optimal
 * strategies for different scenarios:
 * - No screen intersection: Use primary screen fallback
 * - Single screen intersection: Directly capture relative area of one screen
 * - Multiple screen intersection: Capture fragments from each screen and compose
 */
QPixmap ScreenGrabber::grabX11Screenshot(bool &ok, const QRect &rect)
{
    qCDebug(dsrApp) << "Grabbing X11 screenshot for rect:" << rect;
    const QList<QScreen*> intersectingScreens = findIntersectingScreens(rect);
    
    if (intersectingScreens.isEmpty()) {
        qCDebug(dsrApp) << "No intersecting screens found, falling back to primary screen.";
        return grabPrimaryScreenFallback(ok, rect);
    }
    
    if (intersectingScreens.size() == 1) {
        qCDebug(dsrApp) << "One intersecting screen found, grabbing single screen.";
        return grabSingleScreen(ok, rect, intersectingScreens.first());
    }
    
    qCDebug(dsrApp) << "Multiple intersecting screens found, grabbing multiple screens.";
    return grabMultipleScreens(ok, rect, intersectingScreens);
}

/**
 * @brief Find all screens that intersect with the specified rectangular area
 * @param rect The rectangular area to check
 * @return List of screens intersecting with the area
 * 
 * This function iterates through all screens in the system and checks whether
 * their geometric areas intersect with the specified screenshot region.
 * This is the foundation for multi-screen screenshot handling.
 */
QList<QScreen*> ScreenGrabber::findIntersectingScreens(const QRect &rect)
{
    qCDebug(dsrApp) << "Finding intersecting screens for rect:" << rect;
    QList<QScreen*> result;
    const QList<QScreen*> allScreens = QGuiApplication::screens();
    
    for (QScreen *screen : allScreens) {
        if (screen && screen->geometry().intersects(rect)) {
            qCDebug(dsrApp) << "Screen intersects:" << screen->name();
            result.append(screen);
        }
    }
    
    qCDebug(dsrApp) << "Found" << result.size() << "intersecting screens.";
    return result;
}

/**
 * @brief Primary screen fallback screenshot solution
 * @param ok Output parameter indicating screenshot success
 * @param rect The rectangular area to capture
 * @return The captured image
 * 
 * When the screenshot area doesn't intersect with any screen (such as gap areas
 * in irregular screen layouts), use the primary screen for screenshots as a
 * fallback. This typically occurs in multi-screen environments where the
 * coordinate system has "holes".
 */
QPixmap ScreenGrabber::grabPrimaryScreenFallback(bool &ok, const QRect &rect)
{
    qCDebug(dsrApp) << "Performing primary screen fallback grab for rect:" << rect;
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    if (!primaryScreen) {
        qCWarning(dsrApp) << "Primary screen not found, fallback failed.";
        ok = false;
        return {};
    }
    
    qCDebug(dsrApp) << "Grabbing window from primary screen.";
    return primaryScreen->grabWindow(0, rect.x(), rect.y(), rect.width(), rect.height());
}

/**
 * @brief Capture screenshot from a single screen
 * @param ok Output parameter indicating screenshot success
 * @param rect The rectangular area to capture
 * @param screen The target screen to capture from
 * @return The captured image
 * 
 * Optimized path for single-screen screenshots. Converts global coordinates
 * to screen-relative coordinates for efficient capture.
 */
QPixmap ScreenGrabber::grabSingleScreen(bool &ok, const QRect &rect, QScreen *screen)
{
    qCDebug(dsrApp) << "Grabbing single screen:" << screen->name() << " for rect:" << rect;
    Q_UNUSED(ok)
    
    const QRect screenGeometry = screen->geometry();
    const QRect relativeRect(
        rect.x() - screenGeometry.x(),
        rect.y() - screenGeometry.y(),
        rect.width(),
        rect.height()
    );
    
    return screen->grabWindow(0, relativeRect.x(), relativeRect.y(), 
                            relativeRect.width(), relativeRect.height());
}

/**
 * @brief Multi-screen composite screenshot
 * @param ok Output parameter indicating screenshot success
 * @param rect The complete rectangular area to capture
 * @param screens List of involved screens
 * @return The composed complete screenshot
 * 
 * This function handles screenshot requests spanning multiple screens:
 * 1. Create a black canvas of target size
 * 2. Process each intersecting screen individually
 * 3. Calculate relative coordinates and intersection areas for each screen
 * 4. Draw each screen's screenshot fragment to the correct position
 * 5. Return the composed complete image
 * 
 * This solves screenshot issues with irregular multi-screen layouts like 1080p+4K.
 */
QPixmap ScreenGrabber::grabMultipleScreens(bool &ok, const QRect &rect, const QList<QScreen*> &screens)
{
    qCDebug(dsrApp) << "Grabbing multiple screens for rect:" << rect << ", total screens:" << screens.size();
    Q_UNUSED(ok)
    
    QPixmap result(rect.size());
    result.fill(Qt::black);
    QPainter painter(&result);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    
    for (QScreen *screen : screens) {
        const QRect intersection = rect.intersected(screen->geometry());
        if (intersection.isEmpty()) {
            qCDebug(dsrApp) << "Intersection empty for screen:" << screen->name();
            continue;
        }
        
        const QPixmap screenFragment = grabScreenFragment(screen, intersection);
        if (!screenFragment.isNull()) {
            qCDebug(dsrApp) << "Drawing fragment from screen:" << screen->name() << " at position: (" << (intersection.x() - rect.x()) << "," << (intersection.y() - rect.y()) << ").";
            const QPoint destPos(
                intersection.x() - rect.x(),
                intersection.y() - rect.y()
            );
            painter.drawPixmap(destPos, screenFragment);
        }
    }
    
    qCDebug(dsrApp) << "Finished composing multiple screen shot.";
    return result;
}

/**
 * @brief Capture a specific fragment from a screen
 * @param screen The source screen
 * @param intersection The area to capture (in global coordinates)
 * @return The captured screen fragment
 * 
 * Helper function to extract a specific rectangular region from a screen,
 * handling coordinate conversion from global to screen-relative coordinates.
 */
QPixmap ScreenGrabber::grabScreenFragment(QScreen *screen, const QRect &intersection)
{
    qCDebug(dsrApp) << "Grabbing fragment from screen:" << screen->name() << " for intersection:" << intersection;
    const QRect screenGeometry = screen->geometry();
    const QRect relativeRect(
        intersection.x() - screenGeometry.x(),
        intersection.y() - screenGeometry.y(),
        intersection.width(),
        intersection.height()
    );
    
    return screen->grabWindow(0, relativeRect.x(), relativeRect.y(), 
                            relativeRect.width(), relativeRect.height());
}