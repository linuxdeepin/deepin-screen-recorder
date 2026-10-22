// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tablerecognizetoast.h"

#include "../putils.h"

#include <DBlurEffectWidget>
#include <DDialogCloseButton>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DIconButton>
#include <DPushButton>

#include <QApplication>
#include <QCloseEvent>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPainter>
#include <QResizeEvent>
#include <QScreen>
#include <QStyleOptionButton>

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
const int ACTION_MIN_WIDTH = 62;
const int ACTION_TEXT_SPACING = 10;
const int ACTION_RADIUS = 6;
const int ACTION_HORIZONTAL_PADDING = 10;
const int ACTION_FILL_ALPHA = 26;          // 10%
const int ACTION_HOVER_FILL_ALPHA = 38;    // 15%
const int ACTION_PRESSED_FILL_ALPHA = 51;  // 20%
const int ACTION_DISABLED_FILL_ALPHA = 13; // 5%
const int ACTION_TEXT_ALPHA = 179;         // 70%
const int ACTION_DISABLED_TEXT_ALPHA = 102; // 40%
const QColor TOAST_MASK(247, 247, 247, 153);
const QColor TOAST_TEXT_COLOR(0, 0, 0, 179);
// 设计稿只给了浅色一版（symbol 名就是 -Light），深色按同一套规则取反：
// 填充 #202020 60%、文字 #FFFFFF 70%
const QColor TOAST_MASK_DARK(32, 32, 32, 153);
const QColor TOAST_TEXT_COLOR_DARK(255, 255, 255, 179);

/**
 * @brief 设计稿的透明行动按钮
 *
 * DTK 默认 DPushButton 使用不透明底板，无法直接得到设计稿的 10% 透明填充。
 * 这里保留 DPushButton 的文字布局、字体、焦点和禁用语义，仅绘制透明底板；
 * hover、pressed、disabled 均通过 QStyleOption 状态计算，不使用 QSS。
 */
class TransparentActionButton : public DPushButton
{
public:
    explicit TransparentActionButton(const QString &text, QWidget *parent = nullptr)
        : DPushButton(text, parent)
    {
    }

    QSize sizeHint() const override
    {
        const int contentWidth = fontMetrics().horizontalAdvance(text());
        return QSize(qMax(ACTION_MIN_WIDTH, contentWidth + ACTION_HORIZONTAL_PADDING * 2),
                     ACTION_HEIGHT);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)

        QStyleOptionButton option;
        initStyleOption(&option);

        const bool dark = DGuiApplicationHelper::toColorType(option.palette)
                == DGuiApplicationHelper::DarkType;
        const bool enabled = option.state & QStyle::State_Enabled;
        const QColor foreground = dark ? Qt::white : Qt::black;

        int fillAlpha = ACTION_FILL_ALPHA;
        int textAlpha = ACTION_TEXT_ALPHA;
        if (!enabled) {
            fillAlpha = ACTION_DISABLED_FILL_ALPHA;
            textAlpha = ACTION_DISABLED_TEXT_ALPHA;
        } else if (option.state & QStyle::State_Sunken) {
            fillAlpha = ACTION_PRESSED_FILL_ALPHA;
        } else if (option.state & QStyle::State_MouseOver) {
            fillAlpha = ACTION_HOVER_FILL_ALPHA;
        }

        QColor fill = foreground;
        fill.setAlpha(fillAlpha);
        QColor textColor = foreground;
        textColor.setAlpha(textAlpha);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        painter.setBrush(fill);
        painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5),
                                ACTION_RADIUS, ACTION_RADIUS);

        // 直接绘制文字，避免 DTK 按钮样式在 pressed 状态下改用深色文字。
        painter.setFont(font());
        painter.setPen(textColor);
        painter.drawText(rect().adjusted(ACTION_HORIZONTAL_PADDING, 0,
                                         -ACTION_HORIZONTAL_PADDING, 0),
                         Qt::AlignCenter, text());
    }
};

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
    : DFloatingMessage(ResidentType, parent)
    , m_status(status)
{
    qCDebug(dsrApp) << "TableRecognizeToast constructed:" << message;
    // 强提示统一常驻，兼容旧调用方传入的 closable，关闭入口固定为“知道了”。
    Q_UNUSED(closable)

    // 表格识别失败属于强提示：不自动消失，由“知道了”显式关闭。
    // ResidentType 会附带 DTK 关闭按钮，这里不需要第二个关闭入口，直接隐藏。
    if (auto *closeButton = findChild<DDialogCloseButton *>()) {
        closeButton->hide();
    }

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
        m_actionButton = new TransparentActionButton(actionText, this);
        m_actionButton->setMinimumSize(ACTION_MIN_WIDTH, ACTION_HEIGHT);
        // 系统级别为 T6 的字体大小, 默认是14 px
        DFontSizeManager::instance()->bind(m_actionButton, DFontSizeManager::T6);
        connect(m_actionButton, &DPushButton::clicked, this, [this]() {
            Q_EMIT actionTriggered();
            close();
        });
        setWidget(m_actionButton);

        // DFloatingMessage 的外层布局只包一个内容 widget，真正的消息布局在内层。
        // 必须从外层内容 widget 取内层布局；直接 findChild 会拿到外层布局并导致崩溃。
        // DTK 在开启动画时会让 setWidget 的按钮插到文字前面，这里统一校正为
        // 图标 -> 文字 -> 10px -> 按钮，避免切换“最佳性能”后布局顺序不一致。
        if (QLayout *outerLayout = layout()) {
            QWidget *messageContent = outerLayout->count() > 0
                    ? outerLayout->itemAt(0)->widget()
                    : nullptr;
            if (auto *messageLayout = messageContent
                    ? qobject_cast<QHBoxLayout *>(messageContent->layout())
                    : nullptr) {
                messageLayout->setSpacing(0);

                QLabel *messageLabel = findChild<QLabel *>();
                int labelIndex = messageLayout->indexOf(messageLabel);
                int actionIndex = messageLayout->indexOf(m_actionButton);
                if (labelIndex >= 0 && actionIndex >= 0 && actionIndex < labelIndex) {
                    QLayoutItem *actionItem = messageLayout->takeAt(actionIndex);
                    --labelIndex;
                    messageLayout->insertItem(labelIndex + 1, actionItem);
                    actionIndex = labelIndex + 1;
                }
                if (labelIndex >= 0 && actionIndex == labelIndex + 1)
                    messageLayout->insertSpacing(actionIndex, ACTION_TEXT_SPACING);
            }
        }
    }

    // 深色/浅色切换：填充与文字颜色跟着 DTK 的主题走
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, [this]() { applyThemeColors(); });
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
    m_backdrop = image;
}

void TableRecognizeToast::makeFloatingOverlay()
{
    Qt::WindowFlags flags = Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint;
    // 与贴图主窗口保持一致：X11 下主窗口是 override-redirect 置顶窗口，
    // 子控件无法越过它绘制，提示条也必须成为同样的顶层窗口才能跳出小贴图范围。
    if (!PUtils::isTreelandMode)
        flags |= Qt::BypassWindowManagerHint;
    setWindowFlags(flags);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
}

void TableRecognizeToast::popupIn(QWidget *host)
{
    if (!host)
        return;

    // 重定位时先隐藏，避免把上一次的提示条自己采进毛玻璃背景。
    if (isVisible())
        hide();

    m_host = host;
    makeFloatingOverlay();
    setParent(host, windowFlags());
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);

    ensurePolished();
    // 按实际样式和字体锁定按钮完整尺寸，避免空间不足时仅按最小宽高压缩，
    // 导致按钮背景或“知道了”文字被父级裁切。常规字号下仍为设计尺寸。
    if (m_actionButton) {
        const QSize actionSizeHint = m_actionButton->sizeHint();
        m_actionButton->setFixedSize(qMax(ACTION_MIN_WIDTH, actionSizeHint.width()),
                                     qMax(ACTION_HEIGHT, actionSizeHint.height()));
    }

    // 卡片设计高度 42，但字体或系统样式变大时不能被写死的高度裁切。
    // 先按内容 sizeHint 调整，再在“设计高度”和内容最小高度之间取较大值锁定。
    const QMargins margins = contentsMargins();
    const int minimumHeight = TOAST_HEIGHT + margins.top() + margins.bottom();
    setMinimumHeight(minimumHeight);
    setMaximumHeight(QWIDGETSIZE_MAX);
    adjustSize();
    setFixedHeight(qMax(minimumHeight, height()));
    updateLayoutPosition();
    prepareBackdrop();

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

void TableRecognizeToast::closeEvent(QCloseEvent *event)
{
    DFloatingMessage::closeEvent(event);
    if (event->isAccepted())
        Q_EMIT closed();
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

void TableRecognizeToast::prepareBackdrop()
{
    DBlurEffectWidget *bg = blurBackground();
    QWidget *host = m_host.data();
    if (!bg || !host)
        return;

    const QRect hostGlobal(host->mapToGlobal(QPoint(0, 0)), host->size());
    const QRect toastGlobal(mapToGlobal(QPoint(0, 0)), size());

    QImage source;
    m_backdropOrigin = QPoint();

    // 贴图本身完整覆盖提示条时，直接复用贴图截图，避免不必要的屏幕抓取。
    if (!m_backdrop.isNull() && hostGlobal.contains(toastGlobal)) {
        source = m_backdrop;
        if (source.size() != host->size())
            source = source.scaled(host->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_backdropOrigin = hostGlobal.topLeft();
    } else {
        // 小贴图下提示条会伸出贴图范围，原截图没有对应像素；显示前先抓屏幕，
        // 并向外扩出一个模糊半径，保证卡片边缘也能取到背景。
        QScreen *screen = QGuiApplication::screenAt(toastGlobal.center());
        if (!screen)
            screen = QGuiApplication::primaryScreen();
        if (screen) {
            const QRect screenGeometry = screen->geometry();
            const QRect captureRect = toastGlobal.adjusted(-TOAST_BLUR_RADIUS, -TOAST_BLUR_RADIUS,
                                                            TOAST_BLUR_RADIUS, TOAST_BLUR_RADIUS)
                                          .intersected(screenGeometry);
            if (!captureRect.isEmpty()) {
                const QPixmap capture = screen->grabWindow(0,
                                                            captureRect.x() - screenGeometry.x(),
                                                            captureRect.y() - screenGeometry.y(),
                                                            captureRect.width(), captureRect.height());
                if (!capture.isNull()) {
                    source = capture.toImage();
                    source.setDevicePixelRatio(screen->devicePixelRatio());
                    m_backdropOrigin = captureRect.topLeft();
                }
            }
        }
    }

    if (source.isNull()) {
        // Treeland 等无法屏幕抓取的场景退回 DTK 自带的窗口内模糊。
        bg->setBlendMode(DBlurEffectWidget::InWindowBlend);
        if (m_blurGroup) {
            m_blurGroup->removeWidget(bg);
            delete m_blurGroup;
            m_blurGroup = nullptr;
        }
        return;
    }

    bg->setBlendMode(DBlurEffectWidget::InWidgetBlend);
    bg->setRadius(TOAST_BLUR_RADIUS);
    if (!m_blurGroup)
        m_blurGroup = new DBlurEffectGroup;
    m_blurGroup->setSourceImage(source, TOAST_BLUR_RADIUS);
    updateBlurOffset();
}

void TableRecognizeToast::updateBlurOffset()
{
    DBlurEffectWidget *bg = blurBackground();
    if (!m_blurGroup || !bg)
        return;
    m_blurGroup->addWidget(bg, mapToGlobal(QPoint(0, 0)) - m_backdropOrigin);
}

void TableRecognizeToast::updateLayoutPosition()
{
    QWidget *host = m_host.data();
    if (!host)
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

    // 提示条自身是顶层浮层，move() 使用屏幕坐标；只把阴影预留边距向外扩。
    move(QPoint(cardX, cardY) - QPoint(margins.left(), margins.top()));
    updateBlurOffset();
}

TableRecognizeToast::~TableRecognizeToast()
{
    if (m_blurGroup) {
        if (DBlurEffectWidget *bg = blurBackground())
            m_blurGroup->removeWidget(bg);
        delete m_blurGroup;
        m_blurGroup = nullptr;
    }
    qCDebug(dsrApp) << "TableRecognizeToast destructed";
}
