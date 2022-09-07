// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "subtoolwidget.h"
#include "accessibility/acTextDefine.h"

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
    m_ocrButton = new DPushButton(this);
    m_ocrButton->setObjectName(AC_SUBTOOLWIDGET_PIN_OCR_BUT);
    m_ocrButton->setAccessibleName(AC_SUBTOOLWIDGET_PIN_OCR_BUT);
    m_ocrButton->setIconSize(QSize(36, 36));
    m_ocrButton->setMaximumSize(42, 40);
    m_ocrButton->setIcon(QIcon::fromTheme("ocr-normal"));
    m_ocrButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    m_ocrButton->setToolTip(tr("Extract Text"));
    connect(m_ocrButton, SIGNAL(clicked()), this, SIGNAL(signalOcrButtonClicked()));

    // 选项按钮
    m_pinOptionButton = new DPushButton(this);
    m_pinOptionButton->setObjectName(AC_SUBTOOLWIDGET_PIN_OPTION_BUT);
    m_pinOptionButton->setAccessibleName(AC_SUBTOOLWIDGET_PIN_OPTION_BUT);
    DFontSizeManager::instance()->bind(m_pinOptionButton, DFontSizeManager::T8);
    m_pinOptionButton->setText(tr("Options"));
    m_pinOptionButton->setMinimumSize(QSize(60, 40));
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
    m_SaveInfo.first = CLIPBOARD; // 默认保存路径
    m_SaveInfo.second = PNG; // 默认保存格式
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
    });

    //工具栏布局
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setSizeConstraint(QLayout::SetFixedSize);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    hLayout->setMargin(0);
    hLayout->setSpacing(3);
    hLayout->addWidget(m_ocrButton, 0,  Qt::AlignCenter);
    hLayout->addSpacing(4);
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
