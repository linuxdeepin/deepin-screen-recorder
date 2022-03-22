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
#include "utils.h"
#include "accessibility/acTextDefine.h"
#include <QBitmap>
#include <DBlurEffectWidget>
#include <DWindowManagerHelper>
#include <DPlatformWindowHandle>

const int SPACING  = 30; //工具栏与保存按钮之间的间隔

ToolBar::ToolBar(DWidget *parent): QObject(parent)
{
    m_toolbarWidget = nullptr;
    m_saveButton = nullptr;
    m_btWidget = nullptr;
    initToolBar(parent); // 初始化工具栏
}

void ToolBar::initToolBar(DWidget *parent)
{
    m_toolbarWidget = new ToolBarWidget(); // 初始化工具栏
    connect(m_toolbarWidget, SIGNAL(signalOcrButtonClicked()), this, SIGNAL(sendOcrButtonClicked())); //发送OCR点击信号
    connect(m_toolbarWidget, SIGNAL(signalCloseButtonClicked()), this, SIGNAL(sendCloseButtonClicked()));// 发送关闭按钮点击信号

    m_btWidget = new QWidget();
    if (Utils::isWaylandMode) {
        m_btWidget->setWindowFlags(Qt::Sheet | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
    } else {
        m_btWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
    }
    DPlatformWindowHandle handle(m_btWidget);
    handle.setEnableBlurWindow(true);
    //初始化保存按钮
    m_saveButton = new DPushButton(m_btWidget);

    m_saveButton->setObjectName(AC_MAINWINDOW_PIN_SAVE_BUT);
    m_saveButton->setAccessibleName(AC_MAINWINDOW_PIN_SAVE_BUT);
    m_saveButton->setFocusPolicy(Qt::NoFocus);
    m_saveButton->setIconSize(QSize(38, 38));
    m_saveButton->setFixedSize(76, 60);
    m_saveButton->setIcon(QIcon(":/newUI/checked/screenshot-checked.svg"));
    connect(m_saveButton, SIGNAL(clicked()), this, SIGNAL(sendSaveButtonClicked())); // 保存按钮被点击

    DPalette pa;
    pa = m_saveButton->palette();
    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(DPalette::Dark, QColor(0, 129, 255, 204));
    pa.setColor(DPalette::Light, QColor(0, 129, 255, 204));
    m_saveButton->setPalette(pa);
    m_saveButton->setProperty("isShotState", true);

    // 为m_btWidget设置背景色，不然在2D模式下是黑色边框
    DPalette pa2;
    pa2 = m_btWidget->palette();
    pa2.setColor(DPalette::Background, QColor(0, 129, 255, 243));
    m_btWidget->setPalette(pa2);

    QHBoxLayout *hLayout = new QHBoxLayout(m_btWidget);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(m_saveButton, 0, Qt::AlignCenter);
    m_btWidget->setLayout(hLayout);
    m_btWidget->setFixedWidth(76);
}

//显示在点pos
void ToolBar::showAt(QPoint pos, bool isfirstTime)
{
    qDebug() << "pos" << pos;
    this->moveAt(pos);
    if (!m_saveButton->isVisible() || (!m_toolbarWidget->isVisible())) {
        m_btWidget->show();
        m_toolbarWidget->show();
    }
    //隐藏第一次工具栏的显示
    if (isfirstTime) {
        m_btWidget->hide();
        m_toolbarWidget->hide();
    }
}

// 设置隐藏
void ToolBar::setHiden(bool value)
{
    if (value) {
        m_btWidget->hide();
        m_toolbarWidget->hide();
    } else {
        m_btWidget->show();
        m_toolbarWidget->show();
    }
}

//快捷键显示选项菜单
void ToolBar::shortcutOpoints()
{
    m_btWidget->show();
    m_toolbarWidget->show();
    m_toolbarWidget->onOptionButtonClicked();
}

// 获取保存信息
QPair<int, int> ToolBar::getSaveInfo()
{
    return m_toolbarWidget->getSaveInfo();
}
//宽
int ToolBar::toolBarWidth()
{
    return  m_toolbarWidget->width() + SPACING + m_btWidget->width();
}
//高
int ToolBar::toolBarHeight()
{
    return qMax(m_toolbarWidget->height(), m_btWidget->height());
}
//移动位置
void ToolBar::moveAt(QPoint pos)
{
    m_toolbarWidget->move(pos);
    m_btWidget->move(pos.x() + m_toolbarWidget->width() + SPACING, pos.y());
}
//关闭工具栏与保存按钮
void ToolBar::close()
{
    m_toolbarWidget->close();
    m_btWidget->close();
}
//是否为隐藏状态
bool ToolBar::isHidden()
{
    return m_toolbarWidget->isHidden() &&  m_btWidget->isHidden();
}
//是否为活动窗口
bool ToolBar::isActiveWindow()
{
    return m_toolbarWidget->isActiveWindow() ||  m_btWidget->isActiveWindow();
}
