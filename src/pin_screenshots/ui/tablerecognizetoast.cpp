// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tablerecognizetoast.h"

#include "../putils.h"

#include <DBlurEffectWidget>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DIconButton>
#include <DPushButton>

#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QIcon>
#include <QLabel>
#include <QResizeEvent>
#include <QScreen>

#include "../../utils/log.h"

namespace {
// 数值逐项取自设计稿 symbol“DTK/通知与提醒/应用通知/提醒-Light”及其实例：
// 高 42、圆角 12、背景模糊 radius 30、fill rgba(247,247,247,0.60)；
// 图标 16x16、文字 14px、行动按钮高 30、右侧内缩 6。
const int TOAST_HEIGHT = 42;
const int TOAST_RADIUS = 12;
const int TOAST_BLUR_RADIUS = 30;
const int EDGE_MARGIN = 10;
const int ICON_SIZE = 16;
const int ACTION_HEIGHT = 30;
const QColor TOAST_MASK(247, 247, 247, 153);
const QColor TOAST_TEXT_COLOR(0, 0, 0, 179);
// 设计稿只给了浅色一版（symbol 名就是 -Light），深色按同一套规则取反：
// 填充 #202020 60%、文字 #FFFFFF 70%
const QColor TOAST_MASK_DARK(32, 32, 32, 153);
const QColor TOAST_TEXT_COLOR_DARK(255, 255, 255, 179);

// 设计稿 shadow：blur 20 / offsetY 8 / rgba(0,0,0,0.10)
const int TOAST_SHADOW_BLUR = 20;
const int TOAST_SHADOW_OFFSET_Y = 8;
const QColor TOAST_SHADOW_COLOR(0, 0, 0, 26);
} // namespace

TableRecognizeToast::TableRecognizeToast(const QString &message,
                                         const QString &actionText,
                                         bool closable,
                                         Status status,
                                         DWidget *parent)
    : DFloatingMessage(closable ? ResidentType : TransientType, parent)
    , m_status(status)
{
    qCDebug(dsrApp) << "TableRecognizeToast constructed:" << message;

    // 设计稿参数：圆角 12、填充 rgba(247,247,247,0.60)、背景模糊 radius 30
    setFramRadius(TOAST_RADIUS);
    setBlurBackgroundEnabled(true);
    if (DBlurEffectWidget *bg = blurBackground()) {
        bg->setRadius(TOAST_BLUR_RADIUS);
    }

    // 设计稿 shadow：blur 20 / offsetY 8 / rgba(0,0,0,0.10)
    auto *shadow = qobject_cast<QGraphicsDropShadowEffect *>(graphicsEffect());
    if (!shadow) {
        shadow = new QGraphicsDropShadowEffect(this);
        setGraphicsEffect(shadow);
    }
    shadow->setBlurRadius(TOAST_SHADOW_BLUR);
    shadow->setXOffset(0);
    shadow->setYOffset(TOAST_SHADOW_OFFSET_Y);
    shadow->setColor(TOAST_SHADOW_COLOR);

    setMessage(message);
    // 设计稿：消息文字 14px、rgba(0,0,0,0.70)。
    // DFloatingMessage 默认用 DTK 的正文档位（比设计稿大一号），这里按设计稿覆盖。
    if (auto *messageLabel = findChild<QLabel *>()) {
        // 系统级别为 T6 的字体大小, 默认是14 px
        DFontSizeManager::instance()->bind(messageLabel, DFontSizeManager::T6);
    }
    applyThemeColors();
    updateStatusIcon();
    if (auto *iconButton = findChild<DIconButton *>())
        iconButton->setIconSize(QSize(ICON_SIZE, ICON_SIZE));

    if (!actionText.isEmpty()) {
        m_actionButton = new DPushButton(actionText, this);
        m_actionButton->setFixedHeight(ACTION_HEIGHT);
        m_actionButton->setMinimumWidth(62);
        // 系统级别为 T6 的字体大小, 默认是14 px
        DFontSizeManager::instance()->bind(m_actionButton, DFontSizeManager::T6);
        connect(m_actionButton, &DPushButton::clicked, this, [this]() {
            Q_EMIT actionTriggered();
            close();
        });
        setWidget(m_actionButton);
    }

    // 深色/浅色切换：填充与文字颜色跟着 DTK 的主题走
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, [this]() { applyThemeColors(); });

    // DTK 通知自身的关闭信号转发成本控件的 closed()
    connect(this, &DFloatingMessage::messageClosed, this, &TableRecognizeToast::closed);
    connect(this, &DFloatingMessage::closeButtonClicked, this, &TableRecognizeToast::closed);
}

void TableRecognizeToast::applyThemeColors()
{
    if (DBlurEffectWidget *bg = blurBackground())
        bg->setMaskColor(PUtils::themeColor(TOAST_MASK, TOAST_MASK_DARK));

    if (auto *messageLabel = findChild<QLabel *>()) {
        QPalette messagePalette = messageLabel->palette();
        messagePalette.setColor(QPalette::WindowText,
                                PUtils::themeColor(TOAST_TEXT_COLOR, TOAST_TEXT_COLOR_DARK));
        messageLabel->setPalette(messagePalette);
    }
}

void TableRecognizeToast::setBackdropImage(const QImage &image)
{
    // DFloatingMessage 内部的 DBlurEffectWidget 直接取宿主窗口内容做模糊
    Q_UNUSED(image);
}

void TableRecognizeToast::popupIn(QWidget *host)
{
    if (!host)
        return;

    m_host = host;
    if (parentWidget() != host)
        setParent(host);

    ensurePolished();
    // 卡片实体高 42，其余是 DFloatingWidget 按 style 预留的阴影边距
    const QMargins margins = contentsMargins();
    setFixedHeight(TOAST_HEIGHT + margins.top() + margins.bottom());

    show();
    raise();
    updateLayoutPosition();
    qCDebug(dsrApp) << "TableRecognizeToast shown at" << pos() << "size" << size() << "global" << mapToGlobal(QPoint(0, 0));
}

void TableRecognizeToast::resizeEvent(QResizeEvent *event)
{
    DFloatingMessage::resizeEvent(event);
    updateLayoutPosition();
}

void TableRecognizeToast::updateStatusIcon()
{
    // 失败态使用设计稿的红色叹号图标，其余状态复用主题图标
    if (m_status == Error) {
        setIcon(QIcon(QStringLiteral(":/icons/deepin/builtin/texts/table_error.svg")));
        return;
    }
    setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok")));
}

void TableRecognizeToast::updateLayoutPosition()
{
    QWidget *host = m_host.data();
    if (!host || !isVisible())
        return;

    const QMargins margins = contentsMargins();
    const int cardWidth = width() - margins.left() - margins.right();
    const int cardHeight = height() - margins.top() - margins.bottom();

    const QRect hostRect(host->mapToGlobal(QPoint(0, 0)), host->size());

    QScreen *screen = QApplication::screenAt(hostRect.center());
    if (!screen)
        screen = QApplication::primaryScreen();
    const QRect screenRect = screen ? screen->availableGeometry() : hostRect;

    // 首选：贴选区内部底边居中
    int cardY = hostRect.bottom() - EDGE_MARGIN - cardHeight;
    if (cardHeight + EDGE_MARGIN * 2 > hostRect.height()) {
        // 选区放不下时依次退到选区下方、选区上方、屏幕底部
        if (hostRect.bottom() + EDGE_MARGIN + cardHeight <= screenRect.bottom()) {
            cardY = hostRect.bottom() + EDGE_MARGIN;
        } else if (hostRect.top() - EDGE_MARGIN - cardHeight >= screenRect.top()) {
            cardY = hostRect.top() - EDGE_MARGIN - cardHeight;
        } else {
            cardY = screenRect.bottom() - EDGE_MARGIN - cardHeight;
        }
    }

    int cardX = hostRect.center().x() - cardWidth / 2;
    cardX = qBound(screenRect.left() + EDGE_MARGIN,
                   cardX,
                   qMax(screenRect.left() + EDGE_MARGIN,
                        screenRect.right() - EDGE_MARGIN - cardWidth));
    cardY = qBound(screenRect.top() + EDGE_MARGIN,
                   cardY,
                   qMax(screenRect.top() + EDGE_MARGIN,
                        screenRect.bottom() - EDGE_MARGIN - cardHeight));

    const QPoint cardInParent = parentWidget()
            ? parentWidget()->mapFromGlobal(QPoint(cardX, cardY))
            : QPoint(cardX, cardY);

    move(cardInParent - QPoint(margins.left(), margins.top()));
}

TableRecognizeToast::~TableRecognizeToast()
{
    qCDebug(dsrApp) << "TableRecognizeToast destructed";
}
