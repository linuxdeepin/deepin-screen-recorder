// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "zoomIndicator.h"

#include "../utils/baseutils.h"
#include "../utils/tempfile.h"
#include "../utils.h"
#include "../utils/log.h"

#if (QT_VERSION_MAJOR == 5)
#include <QtOpenGL/QOpenGLWidget>
#elif (QT_VERSION_MAJOR == 6)
#include <QOpenGLWidget>
#endif

#include <QColor>
#include <QCursor>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QRgb>

#include <DPalette>
#include <DGuiApplicationHelper>

namespace {
// 以下尺寸、颜色、字号均取自 Sketch 画板「截图录屏-光标指针」(8F8261D2-...)。
// 放大镜浮窗：预览区 150x105 + 信息区 150x65。
constexpr int PREVIEW_HEIGHT = 105;
constexpr int INFO_HEIGHT = 65;
static_assert(PREVIEW_HEIGHT + INFO_HEIGHT == ZoomIndicator::PANEL_HEIGHT,
              "preview + info height must match the panel height");
constexpr int PREVIEW_RADIUS = 8;

// 以光标为中心取样约 15px 宽的区域，放大 10 倍渲染。
constexpr int SAMPLE_WIDTH = 15;
constexpr int SAMPLE_HEIGHT = 11;

// 中心准心框（1px 白描边，中心缺口由十字分隔线让出）。
constexpr int CENTER_BOX_SIZE = 9;

constexpr int SWATCH_SIZE = 10;
constexpr int SWATCH_TEXT_GAP = 6;
constexpr int FONT_PIXEL_SIZE = 12;

// 预览区 1px 描边：#FFFFFF 19.8%
const QColor PREVIEW_BORDER_COLOR(255, 255, 255, 50);
// 十字分隔线：#FFFFFF 20%
const QColor DIVIDER_COLOR(255, 255, 255, 51);
// 中心准心框：#FFFFFF 50%
const QColor CENTER_BOX_COLOR(255, 255, 255, 128);
// 信息区底色：#000000 49.8%
const QColor INFO_FILL_COLOR(0, 0, 0, 127);
// 信息区描边：#000000 19.8%
const QColor INFO_BORDER_COLOR(0, 0, 0, 50);
// 取样失败时的兜底底色
const QColor PREVIEW_FALLBACK_COLOR(0, 0, 0, 127);

QString toHexString(const QColor &color)
{
    // 取样失败（图像为空或坐标越界）时返回空串，避免把无效色值写成 "#-1-1-1"。
    if (!color.isValid()) {
        return QString();
    }
    return QString("#%1%2%3")
            .arg(color.red(), 2, 16, QChar('0'))
            .arg(color.green(), 2, 16, QChar('0'))
            .arg(color.blue(), 2, 16, QChar('0'))
            .toLower();
}

// 预览区路径：上方圆角，下边缘与信息区平接。
QPainterPath previewPath()
{
    const qreal r = PREVIEW_RADIUS;
    const qreal w = ZoomIndicator::PANEL_WIDTH;
    const qreal h = PREVIEW_HEIGHT;
    QPainterPath path;
    path.moveTo(0, r);
    path.arcTo(0, 0, 2 * r, 2 * r, 180, -90);
    path.lineTo(w - r, 0);
    path.arcTo(w - 2 * r, 0, 2 * r, 2 * r, 90, -90);
    path.lineTo(w, h);
    path.lineTo(0, h);
    path.closeSubpath();
    return path;
}
} // namespace

ZoomIndicator::ZoomIndicator(DWidget *parent, bool isRecord)
    : DLabel(parent)
{
    qCDebug(dsrApp) << "ZoomIndicator constructor called, isRecord:" << isRecord;

    QDBusInterface wmInterface("com.deepin.wm",
                               "/com/deepin/wm",
                               "com.deepin.wm",
                               QDBusConnection::sessionBus());
    if (!wmInterface.isValid()) {
        qCDebug(dsrApp) << "Failed to get multitasking view dbus interface!";
        m_isOpenWM = false;
    } else {
        QDBusReply<bool> reply = wmInterface.call("GetMultiTaskingStatus");
        if (!reply.isValid()) {
            qCDebug(dsrApp) << "Failed to call dbus method to get multitasking view status!";
            m_isOpenWM = false;
        } else {
            m_isOpenWM = reply.value();
            qCDebug(dsrApp) << "Multitasking view status:" << (m_isOpenWM ? "open" : "closed");
        }
    }
    m_isRecord = isRecord;

#ifndef ENABLE_UNIT_TEST
    if (Utils::isWaylandMode && !m_isOpenWM && !m_isRecord) {
        qCDebug(dsrApp) << "Creating ZoomIndicatorGL for Wayland mode, hiding current indicator.";
        m_zoomIndicatorGL = new ZoomIndicatorGL(parent);
        this->hide();
        return;
    }
#endif

    setFixedSize(PANEL_WIDTH, PANEL_HEIGHT);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
}

ZoomIndicator::~ZoomIndicator()
{
    qCDebug(dsrApp) << "ZoomIndicator destructor called.";
#ifndef ENABLE_UNIT_TEST
    if (Utils::isWaylandMode && !m_isOpenWM && m_zoomIndicatorGL && !m_isRecord) {
        qCDebug(dsrApp) << "Deleting ZoomIndicatorGL in Wayland mode.";
        delete m_zoomIndicatorGL;
    }
#endif
}

const QImage &ZoomIndicator::cachedFullscreenImage() const
{
    // getFullscreenPixmap() 每次返回的时候是隐式共享的拷贝，这里按 cacheKey 缓存
    // 转换后的 QImage，避免每次重绘都对整屏图做一次深拷贝。
    const QPixmap pixmap = TempFile::instance()->getFullscreenPixmap();
    const qint64 key = pixmap.cacheKey();
    if (key != m_cachedPixmapKey || m_cachedImage.isNull()) {
        m_cachedImage = pixmap.toImage();
        m_cachedImage.setDevicePixelRatio(1.0);
        m_cachedPixmapKey = key;
    }
    return m_cachedImage;
}

QColor ZoomIndicator::sampleColor() const
{
    const QImage &image = cachedFullscreenImage();
    if (image.isNull() || !image.rect().contains(m_cursorPos)) {
        return QColor();
    }
    return QColor(image.pixel(m_cursorPos));
}

QString ZoomIndicator::currentColorHex() const
{
    return toHexString(sampleColor());
}

void ZoomIndicator::drawPreview(QPainter &painter)
{
    const QPainterPath path = previewPath();
    const QRectF previewRect(0, 0, PANEL_WIDTH, PREVIEW_HEIGHT);

    painter.save();
    painter.setClipPath(path);
    // 放大视图要保持像素边界清晰，这里关闭平滑。
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);

    const QImage &image = cachedFullscreenImage();
    const QRect sourceRect(m_cursorPos.x() - SAMPLE_WIDTH / 2,
                           m_cursorPos.y() - SAMPLE_HEIGHT / 2,
                           SAMPLE_WIDTH, SAMPLE_HEIGHT);
    if (!image.isNull() && image.rect().intersects(sourceRect)) {
        painter.drawImage(previewRect, image, sourceRect);
    } else {
        painter.fillRect(previewRect, PREVIEW_FALLBACK_COLOR);
    }
    painter.restore();

    painter.save();
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(PREVIEW_BORDER_COLOR, 1));
    painter.drawPath(path);
    painter.restore();
}

void ZoomIndicator::drawCrosshair(QPainter &painter)
{
    // 设计稿：水平分隔线 y=53、垂直分隔线 x=75、中心准心框 9x9 位于 (70.5, 48.5)。
    // 1px 线条必须落在半像素上，否则会被平分到相邻两列/行，渲染成模糊的浅色线。
    const qreal lineY = PREVIEW_HEIGHT / 2.0;          // 52.5
    const qreal lineX = PANEL_WIDTH / 2.0 + 0.5;       // 75.5
    const qreal boxLeft = PANEL_WIDTH / 2.0 - 4.5;     // 70.5
    const qreal boxTop = PREVIEW_HEIGHT / 2.0 - 4.0;   // 48.5
    const qreal boxRight = boxLeft + CENTER_BOX_SIZE;  // 79.5
    const qreal boxBottom = boxTop + CENTER_BOX_SIZE;  // 57.5

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(QPen(DIVIDER_COLOR, 1));
    // 水平分隔线，中间让出中心准心框
    painter.drawLine(QPointF(0, lineY), QPointF(boxLeft, lineY));
    painter.drawLine(QPointF(boxRight, lineY), QPointF(PANEL_WIDTH, lineY));
    // 垂直分隔线
    painter.drawLine(QPointF(lineX, 0), QPointF(lineX, boxTop));
    painter.drawLine(QPointF(lineX, boxBottom), QPointF(lineX, PREVIEW_HEIGHT));

    // 中心准心框：1px 描边
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(CENTER_BOX_COLOR, 1));
    painter.drawRect(QRectF(boxLeft, boxTop, CENTER_BOX_SIZE, CENTER_BOX_SIZE));
    painter.restore();
}

void ZoomIndicator::drawInfoPanel(QPainter &painter)
{
    const QRectF infoRect(0, PREVIEW_HEIGHT, PANEL_WIDTH, INFO_HEIGHT);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, false);
    // 问题3.3：使用 QPainterPath 绘制底部圆角矩形（仅底部两角圆角）
    QPainterPath infoPath;
    const qreal radius = 4.0;
    infoPath.moveTo(infoRect.left(), infoRect.top());
    infoPath.lineTo(infoRect.right(), infoRect.top());
    infoPath.lineTo(infoRect.right(), infoRect.bottom() - radius);
    infoPath.quadTo(infoRect.right(), infoRect.bottom(),
                    infoRect.right() - radius, infoRect.bottom());
    infoPath.lineTo(infoRect.left() + radius, infoRect.bottom());
    infoPath.quadTo(infoRect.left(), infoRect.bottom(),
                    infoRect.left(), infoRect.bottom() - radius);
    infoPath.closeSubpath();
    painter.setRenderHint(QPainter::Antialiasing, true);
    // 问题3.2：从全屏缓存图采样信息面板背后的区域，缩小再放大实现模糊效果
    const qreal dpr = devicePixelRatioF();
    const QPoint widgetPos = pos();
    const QRect screenInfoRect(
        qRound(widgetPos.x() * dpr),
        qRound((widgetPos.y() + PREVIEW_HEIGHT) * dpr),
        qRound(PANEL_WIDTH * dpr),
        qRound(INFO_HEIGHT * dpr));
    const QImage &fullImg = cachedFullscreenImage();
    if (fullImg.rect().contains(screenInfoRect) && screenInfoRect.width() > 0 && screenInfoRect.height() > 0) {
        // 缓存模糊结果：仅在取样区域变化时重新生成，避免每次 drawInfoPanel 执行两次 QImage 缩放
        if (m_cachedBlurredImage.isNull() || m_cachedBlurRect != screenInfoRect) {
            QImage bgTile = fullImg.copy(screenInfoRect);
            const int blurScale = 8;
            QImage blurred = bgTile.scaled(
                qMax(1, bgTile.width() / blurScale),
                qMax(1, bgTile.height() / blurScale),
                Qt::KeepAspectRatio, Qt::SmoothTransformation);
            blurred = blurred.scaled(bgTile.width(), bgTile.height(),
                Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_cachedBlurredImage = blurred;
            m_cachedBlurRect = screenInfoRect;
        }
        painter.setClipPath(infoPath);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.drawImage(infoRect.topLeft(), m_cachedBlurredImage);
        painter.setClipping(false);
        // 覆盖层使用半透明黑色作为 scrim 暗化背景，保证文字可读性
        painter.fillPath(infoPath, QColor(0, 0, 0, 80));
    } else {
        painter.fillPath(infoPath, INFO_FILL_COLOR);
    }
    // 边框颜色使用 DPalette 主题适配
    DPalette palette = DGuiApplicationHelper::instance()->standardPalette(
        DGuiApplicationHelper::instance()->themeType());
    QColor borderColor = palette.color(DPalette::FrameBorder);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(borderColor, 2));
    painter.drawPath(infoPath);

    QFont font = painter.font();
    font.setPixelSize(FONT_PIXEL_SIZE);
    painter.setFont(font);
    painter.setPen(QColor(Qt::white));

    // 坐标：(X,Y)
    const QRectF coordRect(0, PREVIEW_HEIGHT + 4, PANEL_WIDTH, 17);
    painter.drawText(coordRect, Qt::AlignCenter,
                     QString("(%1,%2)").arg(m_screenPos.x()).arg(m_screenPos.y()));

    // 颜色行：色块 + HEX，整体水平居中
    const QColor color = sampleColor();
    const QString hex = toHexString(color);
    const QFontMetrics metrics(font);
    const qreal textWidth = metrics.horizontalAdvance(hex);
    const qreal rowWidth = SWATCH_SIZE + SWATCH_TEXT_GAP + textWidth;
    const qreal rowLeft = (PANEL_WIDTH - rowWidth) / 2.0;
    const qreal rowTop = PREVIEW_HEIGHT + 23;

    const QRectF swatchRect(rowLeft, PREVIEW_HEIGHT + 28, SWATCH_SIZE, SWATCH_SIZE);
    if (color.isValid()) {
        painter.fillRect(swatchRect, color);
    }
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(Qt::white), 1));
    painter.drawRect(swatchRect);

    painter.setPen(QColor(Qt::white));
    painter.drawText(QRectF(rowLeft + SWATCH_SIZE + SWATCH_TEXT_GAP, rowTop, textWidth, 17),
                     Qt::AlignCenter, hex);

    // 底部操作提示
    const QRectF tipRect(0, PREVIEW_HEIGHT + 42, PANEL_WIDTH, 17);
    painter.drawText(tipRect, Qt::AlignCenter, tr("Press Ctrl+c to copy color"));

    painter.restore();
}

void ZoomIndicator::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    drawPreview(painter);
    drawCrosshair(painter);
    drawInfoPanel(painter);
}

void ZoomIndicator::showMagnifier(QPoint pos)
{
#ifndef ENABLE_UNIT_TEST
    if (Utils::isWaylandMode && !m_isOpenWM && !m_isRecord) {
        m_zoomIndicatorGL->showMagnifier(pos);
        return;
    }
#endif

    this->move(pos);
    this->show();
    this->update();
}

void ZoomIndicator::hideMagnifier()
{
#ifndef ENABLE_UNIT_TEST
    if (Utils::isWaylandMode && !m_isOpenWM && !m_isRecord) {
        qCDebug(dsrApp) << "Hiding magnifier in Wayland mode";
        m_zoomIndicatorGL->makeCurrent();
        m_zoomIndicatorGL->hide();
        return;
    }
#endif
    qCDebug(dsrApp) << "Hiding magnifier";
    this->hide();
}

void ZoomIndicator::setCursorPos(QPoint pos)
{
    m_cursorPos = pos;
}

void ZoomIndicator::setScreenPos(QPoint pos)
{
    m_screenPos = pos;
}
