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
#include "toolbar.h"

#define AC_MAINWINDOW_MAINSHOTBTN "main_shot_but" // 截图按钮
const QSize MIN_ToolBar_SIZE = QSize(344, 60);

ToolBar::ToolBar(DWidget *parent): DLabel(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool); // 设置窗口样式
    setAttribute(Qt::WA_TranslucentBackground, true); //设置透明
    m_toolbarWidget = nullptr;
    m_saveButton = nullptr;
    initToolBar(); // 初始化工具栏
}

void ToolBar::initToolBar()
{
    m_toolbarWidget = new ToolBarWidget(this); // 初始化工具栏
    connect(m_toolbarWidget, SIGNAL(signalOcrButtonClicked()), this, SIGNAL(sendOcrButtonClicked())); //发送OCR点击信号
    connect(m_toolbarWidget, SIGNAL(signalCloseButtonClicked()), this, SIGNAL(sendCloseButtonClicked()));// 发送关闭按钮点击信号

    //初始化保存按钮
    m_saveButton = new DPushButton(this);
    m_saveButton->setFocusPolicy(Qt::NoFocus);
    m_saveButton->setIconSize(QSize(30, 30));
    m_saveButton->setFixedSize(80, 50);
    m_saveButton->setIcon(QIcon(":/newUI/checked/screenshot-checked.svg"));
    connect(m_saveButton, SIGNAL(clicked()), this, SIGNAL(sendSaveButtonClicked())); // 保存按钮被点击
    m_saveButton->setProperty("isShotState", true);

    DPalette pa;
    pa = m_saveButton->palette();
    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(DPalette::Dark, QColor(0, 129, 255, 204));
    pa.setColor(DPalette::Light, QColor(0, 129, 255, 204));
    m_saveButton->setPalette(pa);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setSizeConstraint(QLayout::SetFixedSize);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(m_toolbarWidget, 0, Qt::AlignCenter);
    hLayout->addSpacing(30);
    hLayout->addWidget(m_saveButton, 0, Qt::AlignCenter);
    setLayout(hLayout);
    setMaximumSize(MIN_ToolBar_SIZE);
}

//显示在点pos
void ToolBar::showAt(QPoint pos)
{
    if (!isVisible())
        this->show();
    //qDebug() << "pos" << pos;
    move(pos);
}

//快捷键显示选项菜单
void ToolBar::shortcutOpoints()
{
    this->show();
    m_toolbarWidget->onOptionButtonClicked();
}

// 获取保存信息
QPair<QString, QString> ToolBar::getSaveInfo()
{
    return m_toolbarWidget->getSaveInfo();
}
