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
 * @brief 设计稿里的“复制为Excel格式”按钮
 *
 * 绘制路径和 QPushButton 自带的一模一样（QStylePainter + QStyleOptionButton +
 * CE_PushButton，交给平台样式画），只改一处：非按下状态用设计稿的蓝色文字。
 * 按下时不碰调色板，于是按下态与普通 DTK 按钮逐像素一致（系统强调色底 +
 * 主题自己的文字色），不会出现“设计稿蓝字压在强调色底上”那种脏紫色。
 */
class TableActionButton : public DPushButton
{
public:
    explicit TableActionButton(const QString &text, QWidget *parent = nullptr)
        : DPushButton(text, parent)
    {
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)
        QStylePainter painter(this);
        QStyleOptionButton option;
        initStyleOption(&option);
        if (!(option.state & QStyle::State_Sunken))
            option.palette.setBrush(QPalette::ButtonText, QBrush(ACTION_TEXT_COLOR));
        painter.drawControl(QStyle::CE_PushButton, option);
    }
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

    // 这一步特有的内容：识别成功插图
    auto *illustration = new DLabel(this);
    illustration->setFixedSize(ILLUSTRATION_SIZE, ILLUSTRATION_SIZE);
    // 设计稿深色卡片没有覆盖插图，浅色/深色用的是同一张“插图/表格识别成功/浅色”
    illustration->setPixmap(QIcon(QStringLiteral(":/icons/deepin/builtin/texts/tableresult_success.svg"))
                                    .pixmap(ILLUSTRATION_SIZE, ILLUSTRATION_SIZE));
    illustration->setAlignment(Qt::AlignCenter);
    illustration->setAccessibleName(QStringLiteral("table_result_illustration"));
    addContent(illustration, Qt::AlignHCenter);

    // 这一步特有的内容：结果文案（正文档位 T6）
    auto *message = new DLabel(tr("Table recognized successfully!"), this);
    message->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(message, DFontSizeManager::T6);
    message->setAccessibleName(QStringLiteral("table_result_message"));
    addContent(message, Qt::AlignHCenter);

    // 按钮行交给 DDialog（和例子里 addButton("取消") / addButton("授权", ...) 同款），
    // 沿用 DDialog 的按钮排布与背景；只有设计稿要求“复制为Excel格式”用蓝色文字。
    addButton(tr("Cancel"));

    auto *copyButton = new TableActionButton(tr("Copy as Excel format"), this);
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
    // 我们额外设过的卡片遮罩色需要跟着重设一次。
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, [this]() { applyThemeColors(); });
}

TableResultDialog::~TableResultDialog()
{
    qCDebug(dsrApp) << "TableResultDialog destructed";
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

    show();
    raise();
    activateWindow();
    setupFrostedBackground();
    applyCloseButtonMask();
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
    if (m_backdrop.isNull() || !host)
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

    QImage source = m_backdrop;
    if (source.size() != host->size())
        source = source.scaled(host->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    m_blurGroup->setSourceImage(source, CARD_BLUR_RADIUS);

    // DBlurEffectGroup::paint 按 widget->geometry() + offset 在截图里取样，
    // 这里把对话框相对贴图窗口的位置算出来。
    const QPoint offset = mapToGlobal(QPoint(0, 0)) - host->mapToGlobal(QPoint(0, 0));
    m_blurGroup->addWidget(blur, offset);

    qCDebug(dsrApp) << "table result dialog frosted background, offset:" << offset
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
