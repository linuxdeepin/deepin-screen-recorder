// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tableglasspanel.h"

#include "../putils.h"
#include "../../utils/log.h"

#include <QGraphicsDropShadowEffect>
#include <DPlatformWindowHandle>
#include <DWindowManagerHelper>

#include <QMoveEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QShowEvent>

int tableShadowMargin(int blurRadius, int offsetY)
{
    if (blurRadius <= 0)
        return 0;
    // QGraphicsDropShadowEffect 的外扩范围 = blurRadius + |offset|
    return blurRadius + qAbs(offsetY) + 4;
}

TableGlassPanel::TableGlassPanel(int blurRadius,
                                 int cornerRadius,
                                 const QColor &maskColor,
                                 const QColor &borderColor,
                                 DWidget *parent)
    : DBlurEffectWidget(parent)
    , m_blurRadius(blurRadius)
    , m_cornerRadius(cornerRadius)
    , m_maskColor(maskColor)
    , m_borderColor(borderColor)
{
    // 毛玻璃全部交给 DTK 现成的 DBlurEffectWidget：模糊半径、圆角、遮罩色都由基类绘制。
    setBlurEnabled(true);
    setMode(DBlurEffectWidget::GaussianBlur);
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    setRadius(blurRadius);
    setMaskColor(maskColor);

    // 多个卡片复用同一张截图做模糊源，走 DTK 的 DBlurEffectGroup。
    m_blurGroup = new DBlurEffectGroup;
}

TableGlassPanel::~TableGlassPanel()
{
    if (m_blurGroup) {
        m_blurGroup->removeWidget(this);
        delete m_blurGroup;
        m_blurGroup = nullptr;
    }
}

void TableGlassPanel::setBackdropImage(const QImage &image)
{
    m_backdrop = image;
    updateBlurSource();
}

void TableGlassPanel::makeFloatingOverlay()
{
    Qt::WindowFlags flags = Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint;
    // 贴图主窗口在 X11 下是 BypassWindowManagerHint 的 override-redirect 置顶窗口，
    // 浮层必须同样绕过窗管才能叠在它之上；这也意味着窗管的模糊/阴影不可用，
    // 因此模糊继续用 DBlurEffectWidget + DBlurEffectGroup，阴影用 DGraphicsGlowEffect。
    if (!PUtils::isTreelandMode)
        flags |= Qt::BypassWindowManagerHint;
    setWindowFlags(flags);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
}

void TableGlassPanel::setBackdropHost(QWidget *host)
{
    m_backdropHost = host;
    updateBlurSource();
}

void TableGlassPanel::setPills(const QVector<Pill> &pills)
{
    m_pills = pills;
    update();
}

void TableGlassPanel::clearPills()
{
    m_pills.clear();
    update();
}

void TableGlassPanel::setHighlightColor(const QColor &color)
{
    m_highlightColor = color;
    update();
}

void TableGlassPanel::setPanelColors(const QColor &maskColor, const QColor &borderColor)
{
    m_maskColor = maskColor;
    m_borderColor = borderColor;
    setMaskColor(maskColor);
    update();
}

void TableGlassPanel::useWindowBackdrop()
{
    // 浮层绕过了窗管，BehindWindowBlend 拿不到窗口背后的内容，这里仍由
    // DBlurEffectWidget + DBlurEffectGroup 提供模糊源（DTK 现成的模糊实现）。
    Q_UNUSED(this);
}

void TableGlassPanel::setSoftShadow(int blurRadius, int offsetY, const QColor &color)
{
    // 控件尺寸 = 卡片 + 四周投影边距；maskPath 同步内缩到卡片区，
    // 投影由 DGraphicsGlowEffect 画在边距里（控件外会被 Qt 裁掉）。
    m_shadowMargin = (blurRadius > 0 && color.alpha() > 0)
            ? tableShadowMargin(blurRadius, offsetY)
            : 0;

    // 投影用 Qt 标准的 QGraphicsDropShadowEffect。
    // DTK 只提供 DGraphicsGlowEffect（外发光：会把源图放大 2*distance 再模糊，
    // 且 draw() 忽略 x/y 偏移），用在半透明毛玻璃卡片上会从边缘透进来形成
    // 一圈套一圈的脏边，所以这里不用它。
    if (m_shadowMargin > 0) {
        auto *effect = new QGraphicsDropShadowEffect;
        effect->setBlurRadius(blurRadius);
        effect->setOffset(0, offsetY);
        effect->setColor(color);
        setGraphicsEffect(effect);
    } else {
        setGraphicsEffect(nullptr);
    }

    // 模糊与遮罩只铺在卡片实体上，投影边距保持透明
    QPainterPath card;
    card.addRoundedRect(QRectF(cardRect()), m_cornerRadius, m_cornerRadius);
    setMaskPath(card);
    setBlurRectXRadius(m_cornerRadius);
    setBlurRectYRadius(m_cornerRadius);
}

QRect TableGlassPanel::cardRect() const
{
    return rect().adjusted(m_shadowMargin,
                           m_shadowMargin,
                           -m_shadowMargin,
                           -m_shadowMargin);
}

void TableGlassPanel::updateBlurSource()
{
    if (!m_blurGroup)
        return;

    QWidget *host = m_backdropHost ? m_backdropHost.data() : window();
    if (m_backdrop.isNull() || !host) {
        // 没有外部截图时回退到 DBlurEffectWidget 的默认行为：
        // 直接从所在窗口的绘制结果里取背景。
        m_blurGroup->removeWidget(this);
        m_blurSource = QImage();
        m_blurSourceRadius = -1;
        update();
        return;
    }

    QImage source = m_backdrop;
    if (source.size() != host->size())
        source = source.scaled(host->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    if (m_blurSource.cacheKey() != source.cacheKey() || m_blurSourceRadius != m_blurRadius) {
        m_blurSource = source;
        m_blurSourceRadius = m_blurRadius;
        m_blurGroup->setSourceImage(source, m_blurRadius);
    }
    updateBlurOffset();
}

void TableGlassPanel::updateBlurOffset()
{
    if (!m_blurGroup || m_blurSource.isNull())
        return;

    QWidget *host = m_backdropHost ? m_backdropHost.data() : window();
    if (!host)
        return;

    // DBlurEffectGroup::paint 用 widget->geometry() 取样，这里把它换算到截图坐标系：
    // 窗口内子控件 offset 为 0；独立置顶浮层则补偿宿主窗口的屏幕原点。
    const QPoint widgetGlobal = mapToGlobal(QPoint(0, 0));
    const QPoint hostGlobal = host->mapToGlobal(QPoint(0, 0));
    const QPoint offset = (widgetGlobal - hostGlobal) - geometry().topLeft();
    m_blurGroup->addWidget(this, offset);
}

void TableGlassPanel::resizeEvent(QResizeEvent *event)
{
    DBlurEffectWidget::resizeEvent(event);
    QPainterPath card;
    card.addRoundedRect(QRectF(cardRect()), m_cornerRadius, m_cornerRadius);
    setMaskPath(card);
    updateBlurOffset();
}

void TableGlassPanel::moveEvent(QMoveEvent *event)
{
    DBlurEffectWidget::moveEvent(event);
    updateBlurOffset();
}

void TableGlassPanel::showEvent(QShowEvent *event)
{
    DBlurEffectWidget::showEvent(event);
    updateBlurSource();
}

void TableGlassPanel::paintEvent(QPaintEvent *event)
{
    // 背景模糊、圆角、遮罩由 DTK 的 DBlurEffectWidget 绘制
    DBlurEffectWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF outline = QRectF(cardRect());
    QPainterPath card;
    card.addRoundedRect(outline, m_cornerRadius, m_cornerRadius);
    painter.setClipPath(card);

    // 设计稿里的圆角按钮底（DTK/按钮/对话框、DTK/按钮/透明按钮）
    painter.setPen(Qt::NoPen);
    for (const Pill &pill : m_pills) {
        if (!pill.color.isValid() || pill.color.alpha() == 0)
            continue;
        painter.setBrush(pill.color);
        painter.drawRoundedRect(QRectF(pill.rect), pill.radius, pill.radius);
    }

    painter.setClipping(false);
    painter.setBrush(Qt::NoBrush);

    // 设计稿 border(position=1)：白色玻璃高光，压在圆角边缘上
    if (m_highlightColor.isValid() && m_highlightColor.alpha() > 0) {
        painter.setPen(QPen(m_highlightColor, 1));
        painter.drawRoundedRect(outline.adjusted(0.5, 0.5, -0.5, -0.5),
                                m_cornerRadius, m_cornerRadius);
    }
    // 设计稿 border(position=2)：向内 1px 的描边
    if (m_borderColor.isValid() && m_borderColor.alpha() > 0) {
        painter.setPen(QPen(m_borderColor, 1));
        painter.drawRoundedRect(outline.adjusted(1.0, 1.0, -1.0, -1.0),
                                qMax(0, m_cornerRadius - 1), qMax(0, m_cornerRadius - 1));
    }
}
