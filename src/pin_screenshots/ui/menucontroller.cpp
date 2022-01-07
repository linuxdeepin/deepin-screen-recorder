/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     He MingYang <hemingyang@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
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

#include "menucontroller.h"

#include <QApplication>
#include <QStyleFactory>
#include <QDebug>
#include <map>
DWIDGET_USE_NAMESPACE

MenuController::MenuController(QObject *parent)
    : QObject(parent)
{
    initMenuController();
}

void MenuController::initMenuController()
{
    m_menu = new DMenu();
    m_menu->setFocusPolicy(Qt::StrongFocus);
    m_saveAct = new QAction(tr("Save"), this);
    connect(m_saveAct, &QAction::triggered, [ = ] {
        emit saveAction();
    });

    m_closeAct = new QAction(tr("Exit"), this);
    connect(m_closeAct, &QAction::triggered, [ = ] {
        emit closeAction();
    });
    m_menu->addAction(m_saveAct);
    m_menu->addSeparator();
    m_menu->addAction(m_closeAct);

    connect(m_menu, &DMenu::aboutToHide, this, [ = ] {
        emit menuNoFocus();
    });
}

void MenuController::showMenu(QPoint pos)
{
    m_menu->popup(pos);
}

void MenuController::enterEvent(QEvent *e)
{
    Q_UNUSED(e);
    qApp->setOverrideCursor(Qt::ArrowCursor);
}

MenuController::~MenuController()
{
    if (m_menu) {
        delete  m_menu;
        m_menu = nullptr;
    }
    if (m_saveAct) {
        delete  m_saveAct;
        m_saveAct = nullptr;
    }
    if (m_closeAct) {
        delete  m_closeAct;
        m_closeAct = nullptr;
    }
}
