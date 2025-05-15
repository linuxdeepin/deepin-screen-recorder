// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "menucontroller.h"
#include "../../utils/log.h"

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
        qCDebug(dsrApp) << "Save action triggered";
        emit saveAction();
    });

    m_closeAct = new QAction(tr("Exit"), this);
    connect(m_closeAct, &QAction::triggered, [ = ] {
        qCDebug(dsrApp) << "Exit action triggered";
        emit closeAction();
    });
    m_menu->addAction(m_saveAct);
    m_menu->addSeparator();
    m_menu->addAction(m_closeAct);
    qCDebug(dsrApp) << "Menu actions initialized";

    connect(m_menu, &DMenu::aboutToHide, this, [ = ] {
        qCDebug(dsrApp) << "Menu about to hide";
        emit menuNoFocus();
    });
}

void MenuController::showMenu(QPoint pos)
{
    m_menu->popup(pos);
}

#if (QT_VERSION_MAJOR == 5)
void MenuController::enterEvent(QEvent *e)
{
    Q_UNUSED(e);
    qApp->setOverrideCursor(Qt::ArrowCursor);
}
#elif (QT_VERSION_MAJOR == 6)
void MenuController::enterEvent(QEnterEvent *e)
{
    Q_UNUSED(e);
    qApp->setOverrideCursor(Qt::ArrowCursor);
}
#endif

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
