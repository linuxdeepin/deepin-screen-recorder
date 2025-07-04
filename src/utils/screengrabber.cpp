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
#include <algorithm>
#include <QMap>

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
    return grabX11Screenshot(ok, rect, devicePixelRatio);
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
 * @param devicePixelRatio The device pixel ratio
 * @return The captured desktop image
 * 
 * In X11 mode, screen content can be accessed directly. This function intelligently
 * detects how many screens are involved in the screenshot area and uses optimal
 * strategies for different scenarios:
 * - No screen intersection: Use primary screen fallback
 * - Single screen intersection: Directly capture relative area of one screen
 * - Multiple screen intersection: Capture fragments from each screen and compose
 */
QPixmap ScreenGrabber::grabX11Screenshot(bool &ok, const QRect &rect, const qreal devicePixelRatio)
{
    qCDebug(dsrApp) << "Grabbing X11 screenshot for rect:" << rect;
    const QList<QScreen*> intersectingScreens = findIntersectingScreens(rect);
    
    if (intersectingScreens.isEmpty()) {
        qCDebug(dsrApp) << "No intersecting screens found, falling back to primary screen.";
        return grabPrimaryScreenFallback(ok, rect, devicePixelRatio);
    }
    
    if (intersectingScreens.size() == 1) {
        qCDebug(dsrApp) << "One intersecting screen found, grabbing single screen.";
        return grabSingleScreen(ok, rect, intersectingScreens.first(), devicePixelRatio);
    }
    
    qCDebug(dsrApp) << "Multiple intersecting screens found, grabbing multiple screens.";
    return grabMultipleScreens(ok, rect, intersectingScreens, devicePixelRatio);
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
        if (screen) {
            if (screen->geometry().intersects(rect)) {
                qCDebug(dsrApp) << "Screen intersects:" << screen->name();
                result.append(screen);
            } else {
                qCDebug(dsrApp) << "Screen does NOT intersect:" << screen->name();
            }
        }
    }
    
    qCDebug(dsrApp) << "Found" << result.size() << "intersecting screens.";
    return result;
}

/**
 * @brief Primary screen fallback screenshot solution
 * @param ok Output parameter indicating screenshot success
 * @param rect The rectangular area to capture
 * @param devicePixelRatio The device pixel ratio
 * @return The captured image
 * 
 * When the screenshot area doesn't intersect with any screen (such as gap areas
 * in irregular screen layouts), use the primary screen for screenshots as a
 * fallback. This typically occurs in multi-screen environments where the
 * coordinate system has "holes".
 */
QPixmap ScreenGrabber::grabPrimaryScreenFallback(bool &ok, const QRect &rect, const qreal devicePixelRatio)
{
    qCDebug(dsrApp) << "Performing primary screen fallback grab for rect:" << rect;
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    if (!primaryScreen) {
        qCWarning(dsrApp) << "Primary screen not found, fallback failed.";
        ok = false;
        return {};
    }
    
    qCDebug(dsrApp) << "Grabbing window from primary screen.";
    // Qt6's grabWindow automatically handles devicePixelRatio conversion
    // We just need to trust Qt's internal high DPI handling
    QPixmap result = primaryScreen->grabWindow(0, rect.x(), rect.y(), rect.width(), rect.height());
    
    ok = !result.isNull();
    return result;
}

/**
 * @brief Capture screenshot from a single screen
 * @param ok Output parameter indicating screenshot success
 * @param rect The rectangular area to capture
 * @param screen The target screen to capture from
 * @param devicePixelRatio The device pixel ratio
 * @return The captured image
 * 
 * Optimized path for single-screen screenshots. Converts global coordinates
 * to screen-relative coordinates for efficient capture.
 */
QPixmap ScreenGrabber::grabSingleScreen(bool &ok, const QRect &rect, QScreen *screen, const qreal devicePixelRatio)
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
    
    // Qt6's grabWindow automatically handles devicePixelRatio conversion
    QPixmap result = screen->grabWindow(0, relativeRect.x(), relativeRect.y(), 
                                       relativeRect.width(), relativeRect.height());
    // Force fix high DPI multi-screen screenshot issue: ensure returned image has correct size and devicePixelRatio
    if (!result.isNull()) {
        
        // Calculate expected physical size
        QSize expectedPhysicalSize = QSize(
            static_cast<int>(relativeRect.width() * devicePixelRatio),
            static_cast<int>(relativeRect.height() * devicePixelRatio)
        );
        
        QPixmap correctedResult;
        
        // If size matches exactly, just need to set correct devicePixelRatio
        if (result.size() == expectedPhysicalSize) {
            correctedResult = result;
            correctedResult.setDevicePixelRatio(devicePixelRatio);
        } else {
            // If size doesn't match, scale to correct physical size
            correctedResult = result.scaled(expectedPhysicalSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            correctedResult.setDevicePixelRatio(devicePixelRatio);
        }
        
        result = correctedResult;
    }
    
    ok = !result.isNull();
    return result;
}

/**
 * @brief Handle multi-screen screenshots with X11 native capture
 * @param ok Output parameter indicating screenshot success
 * @param rect The rectangular area to capture (in logical coordinates) - may be incorrect from virtualGeometry
 * @param screens List of intersecting screens
 * @param devicePixelRatio The target device pixel ratio for the final result
 * @return The captured desktop image
 */
QPixmap ScreenGrabber::grabMultipleScreens(bool &ok, const QRect &rect, const QList<QScreen*> &screens, const qreal devicePixelRatio)
{
    // Detect screen layout mode
    bool isVerticalLayout = false;
    bool isHorizontalLayout = false;
    bool isCloneMode = false;
    bool isComplexLayout = false;
    
    if (screens.size() > 1) {
        // Check if it's clone mode (all screens have same geometry)
        QRect firstGeometry = screens[0]->geometry();
        bool allSameGeometry = true;
        for (int i = 1; i < screens.size(); ++i) {
            if (screens[i]->geometry() != firstGeometry) {
                allSameGeometry = false;
                break;
            }
        }
        
        if (allSameGeometry) {
            isCloneMode = true;
            qCWarning(dsrApp) << "Detected clone mode - all screens have same geometry:" << firstGeometry;
        } else {
            // Extended mode - analyze screen layout
            int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;
            QList<QRect> geometries;
            
            for (QScreen *screen : screens) {
                QRect geo = screen->geometry();
                geometries.append(geo);
                minX = qMin(minX, geo.x());
                minY = qMin(minY, geo.y());
                maxX = qMax(maxX, geo.x() + geo.width());
                maxY = qMax(maxY, geo.y() + geo.height());
            }
            
            int totalWidth = maxX - minX;
            int totalHeight = maxY - minY;
            double aspectRatio = double(totalHeight) / double(totalWidth);
            
            // Detect layout type
            bool hasVerticalAlignment = false;
            bool hasHorizontalAlignment = false;
            bool hasOverlap = false;
            
            // Check alignment between screens
            for (int i = 0; i < geometries.size(); ++i) {
                for (int j = i + 1; j < geometries.size(); ++j) {
                    QRect rect1 = geometries[i];
                    QRect rect2 = geometries[j];
                    
                    // Check horizontal alignment (Y coordinates overlap)
                    if (rect1.y() < rect2.y() + rect2.height() && rect2.y() < rect1.y() + rect1.height()) {
                        hasHorizontalAlignment = true;
                    }
                    
                    // Check vertical alignment (X coordinates overlap)
                    if (rect1.x() < rect2.x() + rect2.width() && rect2.x() < rect1.x() + rect1.width()) {
                        hasVerticalAlignment = true;
                    }
                    
                    // Check if there's an overlap area
                    if (rect1.intersects(rect2)) {
                        hasOverlap = true;
                    }
                }
            }
            
            qCDebug(dsrApp) << "Layout analysis - AspectRatio:" << aspectRatio 
                             << "VerticalAlign:" << hasVerticalAlignment 
                             << "HorizontalAlign:" << hasHorizontalAlignment 
                             << "Overlap:" << hasOverlap;
            
            // Determine layout type based on analysis
            if (hasOverlap) {
                isComplexLayout = true;
                qCDebug(dsrApp) << "Detected complex layout with overlapping screens";
            } else if (hasVerticalAlignment && !hasHorizontalAlignment) {
                isVerticalLayout = true;
                qCDebug(dsrApp) << "Detected pure vertical layout";
            } else if (hasHorizontalAlignment && !hasVerticalAlignment) {
                isHorizontalLayout = true;
                qCDebug(dsrApp) << "Detected pure horizontal layout";
            } else if (hasVerticalAlignment && hasHorizontalAlignment) {
                // Both vertical and horizontal alignment detected, determine main direction
                if (aspectRatio > 1.2) {
                    isVerticalLayout = true;
                    qCDebug(dsrApp) << "Detected mixed layout, primarily vertical";
                } else if (aspectRatio < 0.8) {
                    isHorizontalLayout = true;
                    qCDebug(dsrApp) << "Detected mixed layout, primarily horizontal";
                } else {
                    isComplexLayout = true;
                    qCDebug(dsrApp) << "Detected complex mixed layout";
                }
            } else {
                // No obvious alignment detected, possibly irregular layout
                isComplexLayout = true;
                qCDebug(dsrApp) << "Detected irregular layout with no clear alignment";
            }
        }
    }
    
    // Process screens based on mode
    QList<QScreen*> sortedScreens = screens;
    QMap<QScreen*, QRect> screenMappings;
    QRect actualDesktopRect;
    
    if (isCloneMode) {
        // Clone mode: each screen keeps original coordinates
        for (QScreen *screen : sortedScreens) {
            screenMappings[screen] = screen->geometry();
        }
        actualDesktopRect = sortedScreens[0]->geometry();
        qCDebug(dsrApp) << "Clone mode - using original screen geometries";
    } else if (isComplexLayout) {
        // Complex layout: keep original coordinate system, no remapping
        int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;
        for (QScreen *screen : sortedScreens) {
            QRect geo = screen->geometry();
            screenMappings[screen] = geo;
            minX = qMin(minX, geo.x());
            minY = qMin(minY, geo.y());
            maxX = qMax(maxX, geo.x() + geo.width());
            maxY = qMax(maxY, geo.y() + geo.height());
        }
        actualDesktopRect = QRect(minX, minY, maxX - minX, maxY - minY);
        qCDebug(dsrApp) << "Complex layout - preserving original coordinate system";
    } else {
        // Simple layout: apply coordinate mapping
        if (isVerticalLayout) {
            // Vertical layout: sort by Y coordinates
            std::sort(sortedScreens.begin(), sortedScreens.end(), [](QScreen *a, QScreen *b) {
                return a->geometry().y() < b->geometry().y();
            });
            
            // Create continuous vertical coordinate mapping
            int logicalY = 0;
            int maxWidth = 0;
            for (QScreen *screen : sortedScreens) {
                QRect originalGeometry = screen->geometry();
                QRect mappedGeometry(originalGeometry.x(), logicalY, 
                                   originalGeometry.width(), originalGeometry.height());
                screenMappings[screen] = mappedGeometry;
                
                qCDebug(dsrApp) << "Vertical mapping - Screen" << screen->name() 
                                 << "Original:" << originalGeometry << "Mapped:" << mappedGeometry;
                
                logicalY += originalGeometry.height();
                maxWidth = qMax(maxWidth, originalGeometry.x() + originalGeometry.width());
            }
            actualDesktopRect = QRect(0, 0, maxWidth, logicalY);
        } else {
            // Horizontal layout: sort by X coordinates
            std::sort(sortedScreens.begin(), sortedScreens.end(), [](QScreen *a, QScreen *b) {
                return a->geometry().x() < b->geometry().x();
            });
            
            // Create continuous horizontal coordinate mapping
            int logicalX = 0;
            int maxHeight = 0;
            for (QScreen *screen : sortedScreens) {
                QRect originalGeometry = screen->geometry();
                QRect mappedGeometry(logicalX, originalGeometry.y(), 
                                   originalGeometry.width(), originalGeometry.height());
                screenMappings[screen] = mappedGeometry;
                
                qCDebug(dsrApp) << "Horizontal mapping - Screen" << screen->name() 
                                 << "Original:" << originalGeometry << "Mapped:" << mappedGeometry;
                
                logicalX += originalGeometry.width();
                maxHeight = qMax(maxHeight, originalGeometry.y() + originalGeometry.height());
            }
            actualDesktopRect = QRect(0, 0, logicalX, maxHeight);
        }
    }
    
    qCDebug(dsrApp) << "Actual desktop rect:" << actualDesktopRect;
    
    // If target area is incomplete, use actual desktop range
    QRect correctedRect = rect;
    if (rect.width() < actualDesktopRect.width() || rect.height() < actualDesktopRect.height()) {
        correctedRect = actualDesktopRect;
        qCDebug(dsrApp) << "Target rect corrected to:" << correctedRect;
    }
    
    // Create result canvas
    QPixmap result(static_cast<int>(correctedRect.width() * devicePixelRatio), 
                   static_cast<int>(correctedRect.height() * devicePixelRatio));
    result.fill(Qt::black);
    result.setDevicePixelRatio(devicePixelRatio);
    
    qCDebug(dsrApp) << "Canvas created - Physical:" << result.size() << "DPR:" << result.devicePixelRatio();
    
    QPainter painter(&result);
    
    // Process each screen
    for (QScreen *screen : sortedScreens) {
        const QRect originalGeometry = screen->geometry();
        const QRect mappedGeometry = screenMappings[screen];
        const QRect intersection = correctedRect.intersected(mappedGeometry);
        
        if (intersection.isEmpty()) {
            qCDebug(dsrApp) << "Screen" << screen->name() << "has no intersection, skipping";
            continue;
        }
        
        qCDebug(dsrApp) << "=== Processing Screen:" << screen->name() << "===";
        qCDebug(dsrApp) << "Original geometry:" << originalGeometry;
        qCDebug(dsrApp) << "Mapped geometry:" << mappedGeometry;
        qCDebug(dsrApp) << "Intersection:" << intersection;
        
        // Capture entire screen
        QPixmap fullScreenCapture = screen->grabWindow(0);
        
        if (fullScreenCapture.isNull()) {
            qCDebug(dsrApp) << "Failed to capture screen" << screen->name();
            continue;
        }
        
        // Calculate relative coordinates within the original screen
        QRect relativeRect;
        if (isCloneMode) {
            // Clone mode: use intersection directly
            relativeRect = QRect(
                intersection.x() - originalGeometry.x(),
                intersection.y() - originalGeometry.y(),
                intersection.width(),
                intersection.height()
            );
        } else if (isComplexLayout) {
            // Complex layout: use original coordinate system directly
            relativeRect = QRect(
                intersection.x() - originalGeometry.x(),
                intersection.y() - originalGeometry.y(),
                intersection.width(),
                intersection.height()
            );
        } else {
            // Simple layout: need to convert back to original coordinates
            relativeRect = QRect(
                intersection.x() - mappedGeometry.x(),
                intersection.y() - mappedGeometry.y(),
                intersection.width(),
                intersection.height()
            );
        }
        
        qCDebug(dsrApp) << "Relative rect in screen:" << relativeRect;
        
        // Convert to physical coordinates for cropping
        qreal screenDPR = fullScreenCapture.devicePixelRatio();
        QRect physicalCropRect(
            static_cast<int>(relativeRect.x() * screenDPR),
            static_cast<int>(relativeRect.y() * screenDPR),
            static_cast<int>(relativeRect.width() * screenDPR),
            static_cast<int>(relativeRect.height() * screenDPR)
        );
        
        // Ensure crop area is within screen bounds
        physicalCropRect = physicalCropRect.intersected(QRect(0, 0, fullScreenCapture.width(), fullScreenCapture.height()));
        
        if (physicalCropRect.isEmpty()) {
            qCDebug(dsrApp) << "Physical crop rect is empty, skipping screen";
            continue;
        }
        
        // Crop screen fragment
        QPixmap fragment = fullScreenCapture.copy(physicalCropRect);
        
        if (fragment.isNull()) {
            qCDebug(dsrApp) << "Failed to crop fragment from screen";
            continue;
        }
        
        // Calculate drawing position on result canvas
        int canvasX = intersection.x() - correctedRect.x();
        int canvasY = intersection.y() - correctedRect.y();
        
        qCDebug(dsrApp) << "Drawing fragment at canvas position:" << canvasX << "," << canvasY;
        
        // Draw fragment to canvas
        painter.drawPixmap(QRect(canvasX, canvasY, intersection.width(), intersection.height()), fragment);
        
        qCDebug(dsrApp) << "Fragment drawn successfully";
    }
    
    painter.end();
    
    qCDebug(dsrApp) << "=== Final Result ===";
    qCDebug(dsrApp) << "Result size:" << result.size() << "DPR:" << result.devicePixelRatio();
    
    ok = true;
    return result;
}
