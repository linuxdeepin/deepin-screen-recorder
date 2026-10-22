// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tableresultdialog.h"

#include <DApplication>
#include <DBlurEffectWidget>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DIconButton>
#include <DPlatformWindowHandle>
#include <DWindowCloseButton>
#include <DLabel>
#include <DPushButton>
#include <DVerticalLine>

#include <QApplication>
#include <QClipboard>
#include <QIcon>
#include <QMimeData>
#include <QHBoxLayout>
#include <QRegion>
#include <QPoint>
#include <QStyleOptionButton>
#include <QStylePainter>
#include <QVBoxLayout>
#include <QScreen>
#include <QWidget>

#include "accessibility/acTextDefine.h"
#include "../putils.h"
#include "../../utils/log.h"

namespace {
// 设计稿里“表格识别成功”插图 100x100
const int ILLUSTRATION_SIZE = 100;
// 设计稿卡片 360x240、左上角应用图标 20x20、按钮左右/下边距 6、按钮间距 6
const int CARD_WIDTH = 360;
const int CARD_HEIGHT = 240;
// 设计稿卡片圆角 12
const int CARD_RADIUS = 12;
const int APP_ICON_SIZE = 20;
const int BUTTON_MARGIN = 6;
const int BUTTON_SPACING = 6;
// 设计稿文案底边到按钮顶边为 20
const int CONTENT_BUTTON_GAP = 20;
// 设计稿卡片背景模糊 radius 45
const int CARD_BLUR_RADIUS = 45;
// 设计稿卡片 fills：浅色 #EEEEEE 80%、深色 #181818 80%
const QColor CARD_MASK_LIGHT(238, 238, 238, 204);
const QColor CARD_MASK_DARK(24, 24, 24, 204);

// 设计稿按钮下标：0 = 取消，1 = 复制为Excel格式
const int COPY_BUTTON_INDEX = 1;
// 设计稿里“复制为Excel格式”文字为 #0081FF
const QColor ACTION_TEXT_COLOR(0, 129, 255);
} // namespace

/**
 * @brief 表格识别结果弹窗按钮
 *
 * 弹窗是 BypassWindowManagerHint 的贴图窗口内浮层，窗管不维护它的激活状态，
 * DTK 会把启用中的普通按钮按 Inactive 色组绘制，看起来像被禁用。
 * 这里在控件仍启用时固定使用 Active 色组；真正禁用时不干预，仍显示禁用态。
 * “复制为Excel格式”额外使用设计稿要求的蓝色文字。
 */
class TableButton : public DPushButton
{
public:
    explicit TableButton(const QString &text, const QColor &textColor = QColor(),
                         QWidget *parent = nullptr)
        : DPushButton(text, parent)
        , m_textColor(textColor)
    {
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)
        QStylePainter painter(this);
        QStyleOptionButton option;
        initStyleOption(&option);
        if (option.state & QStyle::State_Enabled) {
            option.palette.setCurrentColorGroup(QPalette::Active);
            if (m_textColor.isValid() && !(option.state & QStyle::State_Sunken))
                option.palette.setBrush(QPalette::ButtonText, QBrush(m_textColor));
        }
        painter.drawControl(QStyle::CE_PushButton, option);
    }

private:
    QColor m_textColor;
};

TableResultDialog::TableResultDialog(const QString &html, const QImage &backdrop, DWidget *parent)
    : DDialog(parent)
    , m_html(html)
{
    Q_UNUSED(backdrop);
    qCDebug(dsrApp) << "TableResultDialog constructed, html size:" << html.size();

    setObjectName(AC_TABLERESULT_DIALOG_BUT);
    setAccessibleName(AC_TABLERESULT_DIALOG_BUT);

    // 和 DTK 例子里「开始还原」的对话框一模一样：图标、标题、内容、按钮全走 DDialog 接口。
    QIcon appIcon = QIcon::fromTheme(QStringLiteral("deepin-screen-recorder"));
    if (appIcon.isNull())
        appIcon = QApplication::windowIcon();
    setIcon(appIcon);
    setTitle(tr("Table recognition"));

    // 这一步特有的内容：识别成功插图。设计稿提供浅色/深色两张独立资源。
    m_illustration = new DLabel(this);
    m_illustration->setFixedSize(ILLUSTRATION_SIZE, ILLUSTRATION_SIZE);
    m_illustration->setAlignment(Qt::AlignCenter);
    m_illustration->setAccessibleName(QStringLiteral("table_result_illustration"));
    updateIllustration();
    addContent(m_illustration, Qt::AlignHCenter);

    // 这一步特有的内容：结果文案（正文档位 T6）
    auto *message = new DLabel(tr("Table recognized successfully!"), this);
    message->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(message, DFontSizeManager::T6);
    message->setAccessibleName(QStringLiteral("table_result_message"));
    addContent(message, Qt::AlignHCenter);

    // 按钮行仍交给 DDialog 排布；两个按钮都用同一套绘制逻辑，
    // 避免贴图浮层失焦时把启用中的“取消”按钮误画成禁用态。
    auto *cancelButton = new TableButton(tr("Cancel"), QColor(), this);
    cancelButton->setObjectName(QStringLiteral("CancelButton"));
    cancelButton->setAccessibleName(QStringLiteral("table_result_cancel_button"));
    insertButton(0, cancelButton);

    auto *copyButton = new TableButton(tr("Copy as Excel format"), ACTION_TEXT_COLOR, this);
    copyButton->setObjectName(QStringLiteral("ActionButton"));
    copyButton->setAccessibleName(QStringLiteral("table_result_copy_button"));
    insertButton(COPY_BUTTON_INDEX, copyButton);

    // DDialog::insertButton() 会在每个按钮前自动插一条 DVerticalLine（DTK 对话框的分隔线）。
    // 设计稿两个按钮之间只有间隙、没有竖线，这里把分隔线隐藏掉。
    for (auto *line : findChildren<DVerticalLine *>(QStringLiteral("VLine")))
        line->hide();

    // DDialog 的按钮行边距/间距是内部固定的（10/10/10、spacing 5），
    // 设计稿是左右与底部 6、间距 6，这里直接取到那条布局调成稿子的数值。
    if (auto *mainLayout = qobject_cast<QVBoxLayout *>(layout())) {
        if (auto *buttonItem = mainLayout->itemAt(mainLayout->count() - 1)) {
            if (auto *buttonLayout = qobject_cast<QHBoxLayout *>(buttonItem->layout())) {
                buttonLayout->setContentsMargins(BUTTON_MARGIN, 0, BUTTON_MARGIN, BUTTON_MARGIN);
                buttonLayout->setSpacing(BUTTON_SPACING);
            }
        }
        // DDialog 在内容区和按钮行之间默认放了一个 10px spacer，设计稿要求 20px。
        // 只调整这一项，避免 DDialog::setSpacing() 同时改变插图与文案的间距。
        if (mainLayout->count() >= 2) {
            if (auto *gap = mainLayout->itemAt(mainLayout->count() - 2)->spacerItem())
                gap->changeSize(0, CONTENT_BUTTON_GAP, QSizePolicy::Minimum, QSizePolicy::Fixed);
        }
    }

    // 设计稿左上角应用图标 20x20（DTitlebar 默认按 24 画）
    for (auto *iconButton : findChildren<DIconButton *>()) {
        if (iconButton->accessibleName() == QStringLiteral("DTitlebarIconLabel")) {
            iconButton->setIconSize(QSize(APP_ICON_SIZE, APP_ICON_SIZE));
            break;
        }
    }

    // 设计稿卡片尺寸 360x240；DDialog 默认按内容自适应，这里压到稿件尺寸
    setFixedSize(CARD_WIDTH, CARD_HEIGHT);

    connect(this, &DDialog::buttonClicked, this, [this](int index, const QString &) {
        if (index == COPY_BUTTON_INDEX)
            copyToClipboard();
        close();
    });

    // 深色/浅色切换：DDialog 自带的图标、标题、文字、按钮都由 DTK 跟随主题，
    // 卡片遮罩色和独立插图需要跟着重设。
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, [this]() {
                applyThemeColors();
                updateIllustration();
            });
}

TableResultDialog::~TableResultDialog()
{
    qCDebug(dsrApp) << "TableResultDialog destructed";
}

void TableResultDialog::closeEvent(QCloseEvent *event)
{
    DDialog::closeEvent(event);
    if (event->isAccepted())
        Q_EMIT dismissed();
}

void TableResultDialog::setBackdropImage(const QImage &image)
{
    m_backdrop = image;
}

void TableResultDialog::setResultHtml(const QString &html)
{
    m_html = html;
    qCDebug(dsrApp) << "TableResultDialog result updated, html size:" << m_html.size();
}

void TableResultDialog::showOverlay()
{
    // 贴图主窗是 BypassWindowManagerHint 的 override-redirect 置顶窗，
    // 对话框必须带上同样的 flag 才能叠在它之上（不带会被整个盖住）。
    setWindowFlags(windowFlags() | Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint);

    if (QWidget *host = parentWidget()) {
        const QPoint hostCenter = host->mapToGlobal(QPoint(host->width() / 2,
                                                           host->height() / 2));
        move(hostCenter - QPoint(width() / 2, height() / 2));
    }

    prepareBlurBackdrop();
    show();
    raise();
    activateWindow();
    setupFrostedBackground();
    applyCloseButtonMask();
}

void TableResultDialog::prepareBlurBackdrop()
{
    m_blurBackdrop = QImage();
    m_blurBackdropOrigin = QPoint();
    m_blurBackdropFromScreen = false;

    QWidget *host = parentWidget();
    if (m_backdrop.isNull() || !host)
        return;

    const QRect hostGlobal(host->mapToGlobal(QPoint(0, 0)), host->size());
    const QRect dialogGlobal(mapToGlobal(QPoint(0, 0)), size());
    m_blurBackdrop = m_backdrop;
    m_blurBackdropOrigin = hostGlobal.topLeft();

    // 贴图窗口完整覆盖弹窗时，贴图本身就是最准确的模糊源。
    if (hostGlobal.contains(dialogGlobal))
        return;

    // 小尺寸贴图下弹窗会超出贴图范围，原来的模糊源没有对应像素。
    // 在弹窗显示前抓取屏幕背景，并向外多取一个模糊半径，保证整块卡片都有模糊。
    // 重新定位时先隐藏，避免把弹窗自身截进模糊源。
    if (isVisible())
        hide();
    QScreen *screen = QGuiApplication::screenAt(dialogGlobal.center());
    if (!screen)
        screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;

    const QRect screenGeometry = screen->geometry();
    const QRect captureRect = dialogGlobal.adjusted(-CARD_BLUR_RADIUS, -CARD_BLUR_RADIUS,
                                                    CARD_BLUR_RADIUS, CARD_BLUR_RADIUS)
                                      .intersected(screenGeometry);
    if (captureRect.isEmpty())
        return;

    const QPixmap capture = screen->grabWindow(0,
                                                captureRect.x() - screenGeometry.x(),
                                                captureRect.y() - screenGeometry.y(),
                                                captureRect.width(),
                                                captureRect.height());
    if (capture.isNull())
        return;

    m_blurBackdrop = capture.toImage();
    m_blurBackdrop.setDevicePixelRatio(screen->devicePixelRatio());
    m_blurBackdropOrigin = captureRect.topLeft();
    m_blurBackdropFromScreen = true;

    qCDebug(dsrApp) << "table result dialog uses screen backdrop, host:" << hostGlobal
                    << "dialog:" << dialogGlobal << "capture:" << captureRect
                    << "image:" << m_blurBackdrop.size()
                    << "dpr:" << m_blurBackdrop.devicePixelRatio();
}

void TableResultDialog::updateIllustration()
{
    if (!m_illustration)
        return;

    const QString resource = PUtils::isDarkTheme()
            ? QStringLiteral(":/icons/deepin/builtin/texts/tableresult_success_dark.svg")
            : QStringLiteral(":/icons/deepin/builtin/texts/tableresult_success.svg");
    m_illustration->setPixmap(QIcon(resource).pixmap(ILLUSTRATION_SIZE, ILLUSTRATION_SIZE));
    qCDebug(dsrApp) << "table result illustration, dark:" << PUtils::isDarkTheme()
                    << "resource:" << resource;
}

void TableResultDialog::applyThemeColors()
{
    // 设计稿两张卡片只有填充色不同：浅色 #EEEEEE 80%、深色 #181818 80%
    const QColor maskColor = PUtils::themeColor(CARD_MASK_LIGHT, CARD_MASK_DARK);
    if (auto *blur = findChild<DBlurEffectWidget *>(QStringLiteral("DAbstractDialogBlurEffectWidget"))) {
        blur->setMaskColor(maskColor);
        qCDebug(dsrApp) << "table result dialog theme colors, dark:" << PUtils::isDarkTheme()
                        << "mask:" << maskColor.name(QColor::HexArgb);
    } else if (auto *fallback = findChild<DBlurEffectWidget *>()) {
        fallback->setMaskColor(maskColor);
    }
}

// 贴图窗口是 BypassWindowManagerHint 的 override-redirect 窗口，窗管不会给它裁圆角。
// DTK 的 DWindowCloseButton 悬停时要画一块方形背景，这里按卡片圆角把它自己裁一刀。
void TableResultDialog::applyCloseButtonMask()
{
    auto *closeButton = findChild<DWindowCloseButton *>(QStringLiteral("DTitlebarDWindowCloseButton"));
    if (!closeButton)
        return;

    const QSize buttonSize = closeButton->size();
    qCDebug(dsrApp) << "close button mask, size:" << buttonSize
                    << "geometry:" << closeButton->geometry()
                    << "windowRadius:" << DPlatformWindowHandle(this).windowRadius();
    // 放到布局算完之后再做，兜底：尺寸还没算出来时用 titlebar 的默认按钮尺寸。
    const int w = buttonSize.width() > 0 && buttonSize.width() < width() ? buttonSize.width() : CARD_RADIUS * 2;
    const int h = buttonSize.height() > 0 && buttonSize.height() < height() ? buttonSize.height() : CARD_RADIUS * 2;

    QRegion mask(0, 0, w, h);
    const QRegion cornerRect(w - CARD_RADIUS, 0, CARD_RADIUS, CARD_RADIUS);
    const QRegion cornerDisc(QRect(w - 2 * CARD_RADIUS, 0, 2 * CARD_RADIUS, 2 * CARD_RADIUS),
                             QRegion::Ellipse);
    mask -= (cornerRect - cornerDisc);
    closeButton->setMask(mask);
}

void TableResultDialog::setupFrostedBackground()
{
    QWidget *host = parentWidget();
    if (m_blurBackdrop.isNull() || !host)
        return;

    // DDialog 的背景层是它内部这个 DBlurEffectWidget。
    auto *blur = findChild<DBlurEffectWidget *>(QStringLiteral("DAbstractDialogBlurEffectWidget"));
    if (!blur)
        blur = findChild<DBlurEffectWidget *>();
    if (!blur)
        return;

    // 贴图主窗是 BypassWindowManagerHint（override-redirect），窗管不接管它，
    // DDialog 默认的 BehindWindowBlend（setEnableBlurWindow / setWindowBlurAreaByWM，
    // 都是“请窗管来模糊”）因此不生效。改成 DTK 的另一条路：InWidgetBlend +
    // DBlurEffectGroup，把贴图窗口的截图直接交给 DTK 自己高斯模糊。
    blur->setBlendMode(DBlurEffectWidget::InWidgetBlend);
    // 设计稿：blur radius 45
    blur->setRadius(CARD_BLUR_RADIUS);
    // 卡片填充色按主题取设计稿的两版取值，而不是固定浅色
    applyThemeColors();

    if (!m_blurGroup)
        m_blurGroup = new DBlurEffectGroup;

    QImage source = m_blurBackdrop;
    // 贴图截图按宿主窗口的逻辑尺寸缩放；屏幕截图本身已经带 DPR，不能再次缩放。
    if (!m_blurBackdropFromScreen && source.size() != host->size())
        source = source.scaled(host->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    m_blurGroup->setSourceImage(source, CARD_BLUR_RADIUS);

    // DBlurEffectGroup::paint 按 widget->geometry() + offset 在截图里取样，
    // 这里把对话框相对模糊源左上角的位置算出来。
    const QPoint offset = mapToGlobal(QPoint(0, 0)) - m_blurBackdropOrigin;
    m_blurGroup->addWidget(blur, offset);

    qCDebug(dsrApp) << "table result dialog frosted background, offset:" << offset
                    << "source:" << source.size() << "fromScreen:" << m_blurBackdropFromScreen
                    << "radius:" << blur->radius() << "size:" << size();
}

void TableResultDialog::copyToClipboard() const
{
    if (m_html.isEmpty()) {
        qCWarning(dsrApp) << "table html is empty, nothing to copy";
        return;
    }

    // 库返回的结果即为可直接粘贴到表格软件的内容，这里不做额外转换。
    auto *mimeData = new QMimeData();
    mimeData->setHtml(m_html);
    mimeData->setText(m_html);
    QApplication::clipboard()->setMimeData(mimeData);
    qCDebug(dsrApp) << "table result copied to clipboard, html size:" << m_html.size();
}
