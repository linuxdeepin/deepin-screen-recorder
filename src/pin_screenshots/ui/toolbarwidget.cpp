/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     Zhang Wenchao <zhangwenchao@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "toolbarwidget.h"

#include <QActionGroup>
#include <DFontSizeManager>


#define AC_SUBTOOLWIDGET_OCR_BUTTON "orc_button"// 贴图 文字识别工具按钮
#define AC_TOOLBARWIDGET_CLOSE_BUTTON_TOOL "toolbar_close_button"// 关闭按钮
#define AC_SUBTOOLWIDGET_PIN_OPTION_BUT "shot_option_but" // 贴图 选项按钮
#define THEMETYPE 1 // 主题颜色为浅色

const QSize MIN_TOOL_BUTTON_SIZE = QSize(42, 40);
const QSize MIN_TOOLBAR_WIDGET_SIZE = QSize(224, 60);

ToolBarWidget::ToolBarWidget(DWidget *parent): DFloatingWidget(parent)
{
    setBlurBackgroundEnabled(true);
    blurBackground()->setRadius(30);
    blurBackground()->setMode(DBlurEffectWidget::GaussianBlur);
    blurBackground()->setBlurEnabled(true);
    blurBackground()->setBlendMode(DBlurEffectWidget::InWindowBlend);
    setAttribute(Qt::WA_TranslucentBackground, false); //取消透明
    // 初始化主题样式
    if (DGuiApplicationHelper::instance()->themeType() == THEMETYPE) {
        blurBackground()->setMaskColor(QColor(255, 255, 255, 76));
    } else {
        blurBackground()->setMaskColor(QColor(0, 0, 0, 76));
    }

    initToolBarWidget(); // 初始化按钮菜单
    // 工具栏样式跟随系统样式改变
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ToolBarWidget::onThemeTypeChange);
}

// 获取保存信息
QPair<QString, QString> ToolBarWidget::getSaveInfo()
{
    return m_SaveInfo;
}

// 选项按钮被点击
void ToolBarWidget::onOptionButtonClicked()
{
    if (m_optionMenu->isHidden())
        m_pinOptionButton->showMenu();
    else
        m_optionMenu->hide();
}

// 主题变化
void ToolBarWidget::onThemeTypeChange(DGuiApplicationHelper::ColorType themeType)
{
    if (themeType == THEMETYPE) {
        blurBackground()->setMaskColor(QColor(255, 255, 255, 76));

        m_closeButton->setHoverPic(":/newUI/hover/close-hover.svg");
        m_closeButton->setNormalPic(":/newUI/normal/close-normal.svg");

    } else {
        blurBackground()->setMaskColor(QColor(0, 0, 0, 76));

        m_closeButton->setHoverPic(":/newUI/dark/hover/close-hover_dark.svg");
        m_closeButton->setNormalPic(":/newUI/dark/normal/close-normal_dark.svg");
    }
}

void ToolBarWidget::initToolBarWidget()
{
    // ocr按钮
    m_ocrButton = new DPushButton(this);
    m_ocrButton->setIconSize(QSize(35, 35));
    m_ocrButton->setIcon(QIcon::fromTheme("ocr-normal"));
    m_ocrButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    m_ocrButton->setToolTip(tr("Extract Text"));
    connect(m_ocrButton, SIGNAL(clicked()), this, SIGNAL(signalOcrButtonClicked()));

    // 选项按钮
    m_pinOptionButton = new DPushButton(this);
    DFontSizeManager::instance()->bind(m_pinOptionButton, DFontSizeManager::T8);
    m_pinOptionButton->setText(tr("Options"));
    m_pinOptionButton->setMinimumSize(QSize(73, 40));
    m_pinOptionButton->setToolTip(tr("Options"));
    QActionGroup *t_saveGroup = new QActionGroup(this);
    QActionGroup *t_formatGroup = new QActionGroup(this);
    t_saveGroup->setExclusive(true);
    t_formatGroup->setExclusive(true);

    // 选项菜单
    m_optionMenu = new DMenu(this);
    DFontSizeManager::instance()->bind(m_optionMenu, DFontSizeManager::T8);

    QAction *saveTitleAction = new QAction(m_optionMenu);
    QAction *saveToClipAction = new QAction(m_optionMenu);
    QAction *saveToDesktopAction = new QAction(m_optionMenu);
    QAction *saveToPictureAction = new QAction(m_optionMenu);
    QAction *saveToSpecialPath = new QAction(m_optionMenu);

    QAction *formatTitleAction = new QAction(m_optionMenu);
    QAction *pngAction = new QAction(m_optionMenu);
    QAction *jpgAction = new QAction(m_optionMenu);
    QAction *bmpAction = new QAction(m_optionMenu);

    saveTitleAction->setDisabled(true);
    saveTitleAction->setText(tr("Save to"));

    saveToDesktopAction->setText(tr("Desktop"));
    saveToDesktopAction->setCheckable(true);

    saveToPictureAction->setText(tr("Pictures"));
    saveToPictureAction->setCheckable(true);

    saveToSpecialPath->setText(tr("Folder"));
    saveToSpecialPath->setCheckable(true);

    saveToClipAction->setText(tr("Clipboard"));
    saveToClipAction->setCheckable(true);

    t_saveGroup->addAction(saveToClipAction);
    t_saveGroup->addAction(saveToDesktopAction);
    t_saveGroup->addAction(saveToPictureAction);
    t_saveGroup->addAction(saveToSpecialPath);

    formatTitleAction->setDisabled(true);
    formatTitleAction->setText(tr("Format"));
    pngAction->setText(tr("PNG"));
    pngAction->setCheckable(true);
    jpgAction->setText(tr("JPG"));
    jpgAction->setCheckable(true);
    bmpAction->setText(tr("BMP"));
    bmpAction->setCheckable(true);

    t_formatGroup->addAction(pngAction);
    t_formatGroup->addAction(jpgAction);
    t_formatGroup->addAction(bmpAction);

    //保存方式
    m_optionMenu->addAction(saveTitleAction);
    m_optionMenu->addAction(saveToClipAction);
    m_optionMenu->addAction(saveToDesktopAction);
    m_optionMenu->addAction(saveToPictureAction);
    m_optionMenu->addAction(saveToSpecialPath);
    m_optionMenu->addSeparator();
    // 格式
    m_optionMenu->addAction(formatTitleAction);
    m_optionMenu->addAction(pngAction);
    m_optionMenu->addAction(jpgAction);
    m_optionMenu->addAction(bmpAction);
    m_pinOptionButton->setMenu(m_optionMenu);

    saveToClipAction->setChecked(true); //默认
    pngAction->setChecked(true); //默认
    m_SaveInfo.first = saveToClipAction->text(); // 默认保存路径
    m_SaveInfo.second = pngAction->text(); // 默认保存格式
    connect(t_saveGroup, QOverload<QAction *>::of(&QActionGroup::triggered), [ = ](QAction * t_act) {
        m_SaveInfo.first = t_act->text();
    });

    connect(t_formatGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
    [ = ](QAction * t_act) {
        m_SaveInfo.second = t_act->text();
    });

    m_closeButton = new DImageButton(this);
    // 初始化关闭按钮样式
    if (DGuiApplicationHelper::instance()->themeType() == THEMETYPE) {
        m_closeButton->setHoverPic(":/newUI/hover/close-hover.svg");
        m_closeButton->setNormalPic(":/newUI/normal/close-normal.svg");
    } else {
        m_closeButton->setHoverPic(":/newUI/dark/hover/close-hover_dark.svg");
        m_closeButton->setNormalPic(":/newUI/dark/normal/close-normal_dark.svg");
    }
    // 注册按钮点击事件
    connect(m_closeButton, SIGNAL(clicked()), this, SIGNAL(signalCloseButtonClicked()));

    //工具栏布局
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setMargin(0);
    hLayout->setSpacing(2);
    hLayout->addSpacing(10);
    hLayout->addWidget(m_ocrButton, 0,  Qt::AlignCenter);
    hLayout->addSpacing(4);
    hLayout->addWidget(m_pinOptionButton, 0, Qt::AlignCenter);
    hLayout->addSpacing(4);
    hLayout->addWidget(m_closeButton, 0,  Qt::AlignCenter);
    setLayout(hLayout);
    this->setMinimumSize(MIN_TOOLBAR_WIDGET_SIZE);
}
