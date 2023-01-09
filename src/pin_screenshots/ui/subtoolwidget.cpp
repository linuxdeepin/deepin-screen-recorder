// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "subtoolwidget.h"
#include "accessibility/acTextDefine.h"
#include "settings.h"

#include <QActionGroup>
#include <DFontSizeManager>

#define THEMETYPE 1 // 主题颜色为浅色

const QSize MIN_TOOL_BUTTON_SIZE = QSize(42, 40);


SubToolWidget::SubToolWidget(DWidget *parent): DStackedWidget(parent)
{
    initShotLable();
    setCurrentWidget(m_shotSubTool);
}

// 初始化贴图工具栏按钮
void SubToolWidget::initShotLable()
{
    m_shotSubTool = new DLabel(this);
    // ocr按钮
    m_ocrButton = new ToolButton(this);
//    m_ocrButton->setCheckable(false);
//    m_ocrButton->setFlat(true);
//    m_ocrButton->setFocusPolicy(Qt::NoFocus);
    m_ocrButton->setObjectName(AC_SUBTOOLWIDGET_PIN_OCR_BUT);
    m_ocrButton->setAccessibleName(AC_SUBTOOLWIDGET_PIN_OCR_BUT);
    m_ocrButton->setIconSize(QSize(32, 32));
    m_ocrButton->setFixedSize(32, 32);
    m_ocrButton->setIcon(QIcon::fromTheme("ocr-normal"));
    m_ocrButton->setToolTip(tr("Extract Text"));
    connect(m_ocrButton, SIGNAL(clicked()), this, SIGNAL(signalOcrButtonClicked()));

    // 选项按钮
    m_pinOptionButton = new ToolButton(this);
    m_pinOptionButton->setCheckable(false);
    m_pinOptionButton->setFlat(false);
    m_pinOptionButton->setHoverState(false);
    DPalette pa = m_pinOptionButton->palette();
    DGuiApplicationHelper::ColorType t_type = DGuiApplicationHelper::instance()->themeType();
    if (t_type == DGuiApplicationHelper::ColorType::LightType) {
        pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
        pa.setColor(DPalette::Dark, QColor(192, 192, 192, 255));
        pa.setColor(DPalette::Light, QColor(192, 192, 192, 255));
    } else {
        pa.setColor(DPalette::ButtonText, QColor(228, 228, 228, 255));
        pa.setColor(DPalette::Dark, QColor(64, 64, 64, 255));
        pa.setColor(DPalette::Light, QColor(64, 64, 64, 255));
    }
    m_pinOptionButton->setPalette(pa);
    m_pinOptionButton->setFocusPolicy(Qt::NoFocus);
    m_pinOptionButton->setObjectName(AC_SUBTOOLWIDGET_PIN_OPTION_BUT);
    m_pinOptionButton->setAccessibleName(AC_SUBTOOLWIDGET_PIN_OPTION_BUT);
    DFontSizeManager::instance()->bind(m_pinOptionButton, DFontSizeManager::T6);
    m_pinOptionButton->setText(tr("Options"));
    m_pinOptionButton->setMinimumSize(QSize(60, 36));
    m_pinOptionButton->setToolTip(tr("Options"));
    QActionGroup *t_saveGroup = new QActionGroup(this);
    QActionGroup *t_formatGroup = new QActionGroup(this);
    t_saveGroup->setExclusive(true);
    t_formatGroup->setExclusive(true);

    // 选项菜单
    m_optionMenu = new DMenu(this);
    DFontSizeManager::instance()->bind(m_optionMenu, DFontSizeManager::T6);
    connect(m_optionMenu, &DMenu::aboutToShow, this, &SubToolWidget::updateOptionChecked);

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

    m_SavePathActions.insert(CLIPBOARD, saveToClipAction);
    m_SavePathActions.insert(DESKTOP, saveToDesktopAction);
    m_SavePathActions.insert(PICTURES, saveToPictureAction);
    m_SavePathActions.insert(FOLDER, saveToSpecialPath);

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

    m_SaveFormatActions.insert(PNG, pngAction);
    m_SaveFormatActions.insert(JPG, jpgAction);
    m_SaveFormatActions.insert(BMP, bmpAction);

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

    updateOptionChecked();

    connect(t_saveGroup, QOverload<QAction *>::of(&QActionGroup::triggered), [ = ](QAction * t_act) {
        if (t_act == saveToDesktopAction) {
            qDebug() << "save to desktop";
            m_SaveInfo.first = DESKTOP;
        } else if (t_act == saveToPictureAction) {
            qDebug() << "save to picture";
            m_SaveInfo.first = PICTURES;
        } else if (t_act == saveToSpecialPath) {
            qDebug() << "save to path";
            m_SaveInfo.first = FOLDER;
        } else if (t_act == saveToClipAction) {
            qDebug() << "save to clip";
            m_SaveInfo.first = CLIPBOARD;
        }
        Settings::instance()->setSaveOption(m_SaveInfo);
    });

    connect(t_formatGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
    [ = ](QAction * t_act) {
        if (t_act == pngAction) {
            m_SaveInfo.second = PNG;
        } else if (t_act == jpgAction) {
            m_SaveInfo.second = JPG;
        } else if (t_act == bmpAction) {
            m_SaveInfo.second = BMP;
        }
        Settings::instance()->setSaveOption(m_SaveInfo);
    });

    //工具栏布局
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setSizeConstraint(QLayout::SetFixedSize);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(m_ocrButton, 0,  Qt::AlignCenter);
    hLayout->addWidget(m_pinOptionButton, 0, Qt::AlignCenter);
    m_shotSubTool->setLayout(hLayout);
    addWidget(m_shotSubTool);
}

// 获取保存信息
QPair<int, int> SubToolWidget::getSaveInfo()
{
    return m_SaveInfo;
}

// 选项按钮被点击
void SubToolWidget::onOptionButtonClicked()
{
    if (m_optionMenu->isHidden())
        m_pinOptionButton->showMenu();
    else
        m_optionMenu->hide();
}

void SubToolWidget::updateOptionChecked()
{
    QPair<int, int> saveInfo = Settings::instance()->getSaveOption();
    //没有配置文件时，给定一个默认值
    if (saveInfo.first == 0 && saveInfo.second == 0) {
        m_SaveInfo.first = CLIPBOARD; // 默认保存路径
        m_SaveInfo.second = PNG; // 默认保存格式
    } else {
        m_SaveInfo = saveInfo;
    }
    m_SavePathActions.value(m_SaveInfo.first)->setChecked(true);
    m_SaveFormatActions.value(m_SaveInfo.second)->setChecked(true);
}
