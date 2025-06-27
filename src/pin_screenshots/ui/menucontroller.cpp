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
    qCDebug(dsrApp) << "MenuController constructor called.";
    initMenuController();
    qCDebug(dsrApp) << "initMenuController finished.";
}

void MenuController::initMenuController()
{
    qCDebug(dsrApp) << "Initializing MenuController.";
    m_menu = new DMenu();
    qCDebug(dsrApp) << "DMenu object created.";
    m_menu->setFocusPolicy(Qt::StrongFocus);
    m_saveAct = new QAction(tr("Save"), this);
    connect(m_saveAct, &QAction::triggered, [ = ] {
        qCDebug(dsrApp) << "Save action triggered.";
        emit saveAction();
    });

    m_closeAct = new QAction(tr("Exit"), this);
    connect(m_closeAct, &QAction::triggered, [ = ] {
        qCDebug(dsrApp) << "Exit action triggered.";
        emit closeAction();
    });
    m_menu->addAction(m_saveAct);
    m_menu->addSeparator();
    m_menu->addAction(m_closeAct);
    qCDebug(dsrApp) << "Menu actions initialized.";

    connect(m_menu, &DMenu::aboutToHide, this, [ = ] {
        qCDebug(dsrApp) << "Menu about to hide.";
        emit menuNoFocus();
    });
}

void MenuController::showMenu(QPoint pos)
{
    qCDebug(dsrApp) << "Showing menu at position:" << pos << ".";
    m_menu->popup(pos);
}

#if (QT_VERSION_MAJOR == 5)
void MenuController::enterEvent(QEvent *e)
{
    Q_UNUSED(e);
    qCDebug(dsrApp) << "Enter event received (Qt5), setting override cursor.";
    qApp->setOverrideCursor(Qt::ArrowCursor);
}
#elif (QT_VERSION_MAJOR == 6)
void MenuController::enterEvent(QEnterEvent *e)
{
    Q_UNUSED(e);
    qCDebug(dsrApp) << "Enter event received (Qt6), setting override cursor.";
    qApp->setOverrideCursor(Qt::ArrowCursor);
}
#endif

MenuController::~MenuController()
{
    qCDebug(dsrApp) << "MenuController destructor called.";
    if (m_menu) {
        qCDebug(dsrApp) << "Deleting DMenu object.";
        delete  m_menu;
        m_menu = nullptr;
    }
    if (m_saveAct) {
        qCDebug(dsrApp) << "Deleting save action.";
        delete  m_saveAct;
        m_saveAct = nullptr;
    }
    if (m_closeAct) {
        qCDebug(dsrApp) << "Deleting close action.";
        delete  m_closeAct;
        m_closeAct = nullptr;
    }
}
