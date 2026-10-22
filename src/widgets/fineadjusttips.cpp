// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fineadjusttips.h"
#include "utils/log.h"

#include <QFontMetrics>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPen>

DWIDGET_USE_NAMESPACE

namespace {

// 以下尺寸与 Sketch 控件 “提示语/截图时”“提示语/选区完成时” 保持一致
const int PANEL_WIDTH = 395;
const int CURSOR_PANEL_HEIGHT = 44;
const int SELECTION_PANEL_HEIGHT = 112;
const int PANEL_PADDING = 10;
const int KEY_CAP_SIZE = 24;
const int KEY_CAP_RADIUS = 4;
const int KEY_CAP_STEP = 28;   // 单个字符按键 24 + 4 间距
const int ROW_STEP = 34;       // 行高 24 + 10 行距
const int PANEL_RADIUS = 12;

// 黑 40% 底板，白 10% 描边 / 按键底色，白 70% 文字
const QColor PANEL_BACKGROUND(0, 0, 0, 102);
const QColor PANEL_BORDER(255, 255, 255, 26);
const QColor KEY_CAP_BACKGROUND(255, 255, 255, 26);
const QColor TEXT_ENABLED(255, 255, 255, 178);
const QColor TEXT_DISABLED(255, 255, 255, 102);

QFont keyCapFont()
{
    QFont font;
    font.setPixelSize(14);
    font.setWeight(QFont::Medium);
    return font;
}

QFont descriptionFont()
{
    QFont font;
    font.setPixelSize(14);
    font.setWeight(QFont::Normal);
    return font;
}

QColor capBackground(bool enabled)
{
    QColor color = KEY_CAP_BACKGROUND;
    if (!enabled) {
        color.setAlpha(16);
    }
    return color;
}

QColor textColor(bool enabled)
{
    return enabled ? TEXT_ENABLED : TEXT_DISABLED;
}

} // namespace

FineAdjustTips::FineAdjustTips(DWidget *parent)
    : DBlurEffectWidget(parent)
{
    qCDebug(dsrApp) << "FineAdjustTips constructor called.";
    setObjectName("FineAdjustTips");
    setAccessibleName("FineAdjustTips");
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFocusPolicy(Qt::NoFocus);

    // 设计稿：黑色 40% 底板 + 高斯模糊，圆角 12。
    setRadius(30);
    DBlurEffectWidget::setMode(DBlurEffectWidget::GaussianBlur);
    setBlurEnabled(true);
    setBlurRectXRadius(PANEL_RADIUS);
    setBlurRectYRadius(PANEL_RADIUS);
    setMaskColor(PANEL_BACKGROUND);

    dockToBottomLeft();
}

FineAdjustTips::~FineAdjustTips()
{
    qCDebug(dsrApp) << "FineAdjustTips destructor called.";
}

void FineAdjustTips::setMode(Mode mode)
{
    if (m_mode == mode) {
        return;
    }
    m_mode = mode;
    dockToBottomLeft();
    update();
}

FineAdjustTips::Mode FineAdjustTips::mode() const
{
    return m_mode;
}

void FineAdjustTips::setResizeEnabled(bool enabled)
{
    if (m_resizeEnabled == enabled) {
        return;
    }
    m_resizeEnabled = enabled;
    update();
}

bool FineAdjustTips::resizeEnabled() const
{
    return m_resizeEnabled;
}

int FineAdjustTips::contentWidth() const
{
    // 设计稿按中文文案定宽 395；当文案（如未翻译的英文）更长时自动加宽，避免被裁剪
    QFontMetrics metrics(descriptionFont());
    int rightEdge = PANEL_PADDING + 241 + metrics.horizontalAdvance(
                m_mode == SelectionMode ? tr("Move the capture area by 1 pixel")
                                        : tr("Move the mouse pointer by 1 pixel"));
    if (m_mode == SelectionMode) {
        rightEdge = qMax(rightEdge, PANEL_PADDING + 169 + metrics.horizontalAdvance(tr("Enlarge the capture area by 1 pixel")));
        rightEdge = qMax(rightEdge, PANEL_PADDING + 225 + metrics.horizontalAdvance(tr("Shrink the capture area by 1 pixel")));
    }
    return qMax(PANEL_WIDTH, rightEdge + PANEL_PADDING);
}

QSize FineAdjustTips::panelSize() const
{
    const int height = (m_mode == SelectionMode) ? SELECTION_PANEL_HEIGHT : CURSOR_PANEL_HEIGHT;
    return QSize(contentWidth(), height);
}

void FineAdjustTips::dockToBottomLeft()
{
    QWidget *host = parentWidget();
    // 窗口尚未完成布局（宽高为 0）时先不移动，避免停靠到左上角
    if (host == nullptr || host->width() <= 0 || host->height() <= 0) {
        return;
    }
    setFixedSize(panelSize());
    const int x = PANEL_PADDING;
    const int y = qMax(0, host->height() - PANEL_PADDING - height());
    move(x, y);
}

void FineAdjustTips::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    // 模糊底板由 DBlurEffectWidget 绘制；这里只画 1px 描边和前景内容。
    DBlurEffectWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    const QRectF panelRect = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QPainterPath panelPath;
    panelPath.addRoundedRect(panelRect, PANEL_RADIUS, PANEL_RADIUS);
    painter.setPen(QPen(PANEL_BORDER, 1));
    painter.drawPath(panelPath);

    // 设计稿 “编组” 位于面板 (10,10)，按键块从内容区 x=0 开始。
    painter.translate(PANEL_PADDING, 0);

    // 第一行：W A S D / ↑ ↓ ← → 描述
    const int row1Y = PANEL_PADDING;
    static const char *const letterKeys[] = {"W", "A", "S", "D"};
    int keyX = 0;
    for (const char *key : letterKeys) {
        drawKeyCap(painter, QRect(keyX, row1Y, KEY_CAP_SIZE, KEY_CAP_SIZE),
                   QString::fromLatin1(key), true);
        keyX += KEY_CAP_STEP;
    }
    drawSeparator(painter, QRect(111, row1Y + 3, 9, 18), true);
    drawArrowKeys(painter, 123, row1Y, true);
    const QString firstDescription = (m_mode == SelectionMode)
            ? tr("Move the capture area by 1 pixel")
            : tr("Move the mouse pointer by 1 pixel");
    drawDescription(painter, QPoint(241, row1Y), firstDescription, true);

    if (m_mode != SelectionMode) {
        return;
    }

    // 第二行：Ctrl + ↑ ↓ ← → 扩大选区
    const int row2Y = PANEL_PADDING + ROW_STEP;
    drawKeyCap(painter, QRect(0, row2Y, 36, KEY_CAP_SIZE), QStringLiteral("Ctrl"), m_resizeEnabled);
    drawPlus(painter, QRect(39, row2Y + 8, 9, 9), m_resizeEnabled);
    drawArrowKeys(painter, 51, row2Y, m_resizeEnabled);
    drawDescription(painter, QPoint(169, row2Y), tr("Enlarge the capture area by 1 pixel"), m_resizeEnabled);

    // 第三行：Ctrl + Shift + ↑ ↓ ← → 缩小选区
    const int row3Y = PANEL_PADDING + ROW_STEP * 2;
    drawKeyCap(painter, QRect(0, row3Y, 36, KEY_CAP_SIZE), QStringLiteral("Ctrl"), m_resizeEnabled);
    drawPlus(painter, QRect(39, row3Y + 8, 9, 9), m_resizeEnabled);
    drawKeyCap(painter, QRect(51, row3Y, 42, KEY_CAP_SIZE), QStringLiteral("Shift"), m_resizeEnabled);
    drawPlus(painter, QRect(96, row3Y + 8, 9, 9), m_resizeEnabled);
    drawArrowKeys(painter, 108, row3Y, m_resizeEnabled);
    drawDescription(painter, QPoint(225, row3Y), tr("Shrink the capture area by 1 pixel"), m_resizeEnabled);
}

void FineAdjustTips::drawKeyCap(QPainter &painter, const QRect &rect, const QString &text, bool enabled)
{
    QPainterPath path;
    path.addRoundedRect(rect, KEY_CAP_RADIUS, KEY_CAP_RADIUS);
    painter.fillPath(path, capBackground(enabled));

    painter.setFont(keyCapFont());
    painter.setPen(textColor(enabled));
    painter.drawText(rect, Qt::AlignCenter, text);
}

void FineAdjustTips::drawSeparator(QPainter &painter, const QRect &rect, bool enabled)
{
    // 设计稿“图标/截图提示语/或”：9x18 切图。
    painter.save();
    painter.setOpacity(enabled ? 1.0 : 0.57);
    QIcon(QStringLiteral(":/icons/deepin/builtin/texts/shortcut-separator_9px.svg"))
            .paint(&painter, rect);
    painter.restore();
}

void FineAdjustTips::drawPlus(QPainter &painter, const QRect &rect, bool enabled)
{
    // 设计稿“图标/截图提示语/相加”：9x9 切图。
    painter.save();
    painter.setOpacity(enabled ? 1.0 : 0.57);
    QIcon(QStringLiteral(":/icons/deepin/builtin/texts/shortcut-plus_9px.svg"))
            .paint(&painter, rect);
    painter.restore();
}

void FineAdjustTips::drawArrowKeys(QPainter &painter, int x, int y, bool enabled)
{
    // 上/下/左/右方向键符号，避免源码中直接出现非 ASCII 字符
    static const ushort arrowKeys[] = {0x2191, 0x2193, 0x2190, 0x2192};
    for (ushort code : arrowKeys) {
        drawKeyCap(painter, QRect(x, y, KEY_CAP_SIZE, KEY_CAP_SIZE),
                   QString(QChar(code)), enabled);
        x += KEY_CAP_STEP;
    }
}

void FineAdjustTips::drawDescription(QPainter &painter, const QPoint &topLeft, const QString &text, bool enabled)
{
    painter.setFont(descriptionFont());
    painter.setPen(textColor(enabled));
    const QRect box(topLeft.x(), topLeft.y(), width() - topLeft.x() - PANEL_PADDING, KEY_CAP_SIZE);
    painter.drawText(box, Qt::AlignLeft | Qt::AlignVCenter, text);
}
