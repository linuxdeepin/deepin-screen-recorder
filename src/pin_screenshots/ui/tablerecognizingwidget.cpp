// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tablerecognizingwidget.h"

#include "../putils.h"

#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <QIcon>

#include "../../utils/log.h"

namespace {
// 数值逐项取自设计稿画板“截图录屏-工具栏-表格识别2（识别中）”
// 卡片：124x134，圆角 12，背景模糊 radius 30，
// fill rgba(247,247,247,0.60)，border rgba(0,0,0,0.10) 1px
const int CARD_WIDTH = 124;
const int CARD_HEIGHT = 134;
const int CARD_RADIUS = 12;
const int CARD_BLUR_RADIUS = 30;
const QColor CARD_MASK(247, 247, 247, 153);
const QColor CARD_BORDER(0, 0, 0, 26);
// 设计稿只给了浅色一版，深色按同一套取值规则取反：填充 #202020 60%、描边 #FFFFFF 10%
const QColor CARD_MASK_DARK(32, 32, 32, 153);
const QColor CARD_BORDER_DARK(255, 255, 255, 26);
// 设计稿 border(position=1)：白色玻璃高光 rgba(255,255,255,0.30)
const QColor CARD_HIGHLIGHT(255, 255, 255, 77);

// ICON：48x48 @ (38,27.5)
const int SPINNER_SIZE = 48;
const int SPINNER_LEFT = 38;
const int SPINNER_TOP = 28;
// 设计稿 DTK/加载动画/Blue 的圆点颜色
const QColor SPINNER_COLOR(0, 91, 255);

// 文本“表格识别中…”：14px，@ (20,90.5,84,18)，rgba(0,0,0,0.70)
const int TIP_LEFT = 20;
const int TIP_TOP = 90;
const int TIP_WIDTH = 84;
const int TIP_HEIGHT = 18;
const QColor TIP_TEXT_COLOR(0, 0, 0, 179);
const QColor TIP_TEXT_COLOR_DARK(255, 255, 255, 179);

// 设计稿 shadow：blur 20 / offsetY 8 / rgba(0,0,0,0.10)
const int CARD_SHADOW_BLUR = 20;
const int CARD_SHADOW_OFFSET_Y = 8;
const QColor CARD_SHADOW_COLOR(0, 0, 0, 26);
} // namespace

TableRecognizingWidget::TableRecognizingWidget(DWidget *parent)
    : TableGlassPanel(CARD_BLUR_RADIUS, CARD_RADIUS, CARD_MASK, CARD_BORDER, parent)
{
    qCDebug(dsrApp) << "TableRecognizingWidget constructed";
    setHighlightColor(CARD_HIGHLIGHT);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    // 设计稿 shadow：卡片四周需要预留投影空间，卡片实体绘制在 inset 之后。
    // 先按预留边距确定控件尺寸，再设置投影，保证 maskPath 与控件尺寸一致。
    const int inset = tableShadowMargin(CARD_SHADOW_BLUR, CARD_SHADOW_OFFSET_Y);
    setFixedSize(CARD_WIDTH + inset * 2, CARD_HEIGHT + inset * 2);
    setSoftShadow(CARD_SHADOW_BLUR, CARD_SHADOW_OFFSET_Y, CARD_SHADOW_COLOR);

    // 设计稿 ICON 为 DTK/加载动画/Blue：48x48 @ (38,27.5)。
    // 直接使用 DTK 基础控件 DSpinner，并把高亮色设为设计稿的蓝色 #005BFF。
    m_spinner = new DSpinner(this);
    m_spinner->setFixedSize(SPINNER_SIZE, SPINNER_SIZE);
    QPalette spinnerPalette = m_spinner->palette();
    spinnerPalette.setColor(QPalette::Highlight, SPINNER_COLOR);
    m_spinner->setPalette(spinnerPalette);
    m_spinner->move(inset + SPINNER_LEFT, inset + SPINNER_TOP);

    m_tipLabel = new DLabel(tr("Table recognition in progress..."), this);
    m_tipLabel->setGeometry(inset + TIP_LEFT, inset + TIP_TOP, TIP_WIDTH, TIP_HEIGHT);
    m_tipLabel->setAlignment(Qt::AlignCenter);
    // 系统级别为 T6 的字体大小, 默认是14 px
    DFontSizeManager::instance()->bind(m_tipLabel, DFontSizeManager::T6);
    QPalette tipPalette = m_tipLabel->palette();
    tipPalette.setColor(QPalette::WindowText, TIP_TEXT_COLOR);
    m_tipLabel->setPalette(tipPalette);

    // 构造时先按当前主题取一次设计稿取值（浅色/深色两版）
    applyThemeColors();

    // 深色/浅色切换：卡片填充、描边与提示文字跟着 DTK 的主题走
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, [this]() { applyThemeColors(); });
}

void TableRecognizingWidget::applyThemeColors()
{
    setPanelColors(PUtils::themeColor(CARD_MASK, CARD_MASK_DARK),
                   PUtils::themeColor(CARD_BORDER, CARD_BORDER_DARK));

    if (m_tipLabel) {
        QPalette tipPalette = m_tipLabel->palette();
        tipPalette.setColor(QPalette::WindowText,
                            PUtils::themeColor(TIP_TEXT_COLOR, TIP_TEXT_COLOR_DARK));
        m_tipLabel->setPalette(tipPalette);
    }
}

TableRecognizingWidget::~TableRecognizingWidget()
{
    qCDebug(dsrApp) << "TableRecognizingWidget destructed";
}

void TableRecognizingWidget::startOn(QWidget *host)
{
    if (!host)
        return;

    repositionOn(host);
    show();
    raise();
    m_spinner->start();
}

void TableRecognizingWidget::repositionOn(QWidget *host)
{
    if (!host)
        return;

    // 窗口内浮层：永远压在贴图窗口内容之上，且尺寸固定为设计稿大小，
    // 只按宿主窗口居中，不随宿主窗口缩放。
    if (parentWidget() != host)
        setParent(host);
    setBackdropHost(host);
    const int inset = contentMargin();
    const int cardWidth = width() - inset * 2;
    const int cardHeight = height() - inset * 2;
    move((host->width() - cardWidth) / 2 - inset,
         (host->height() - cardHeight) / 2 - inset);
}

void TableRecognizingWidget::stop()
{
    m_spinner->stop();
    hide();
}
