// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolbarhost.h"

#include "../main_window.h"

MainWindowToolBarHost::MainWindowToolBarHost(MainWindow *window, QWidget *anchor)
    : ToolBarHost(anchor)
    , m_window(window)
{
    Q_ASSERT(m_window);
    connect(m_window, &MainWindow::microPhoneEnable,
            this, &ToolBarHost::microphoneEnabled);
    connect(m_window, &MainWindow::setUndoEnable,
            this, &ToolBarHost::undoAvailabilityChanged);
}

void MainWindowToolBarHost::confirm()
{
    m_window->confirm();
}

void MainWindowToolBarHost::close(bool hideToolBar)
{
    if (hideToolBar)
        emit m_window->screenshotSaved(QString());
    m_window->exitApp();
}

void MainWindowToolBarHost::save()
{
    m_window->saveScreenShotToFile();
}

void MainWindowToolBarHost::undo()
{
    m_window->unDo();
}

void MainWindowToolBarHost::undoHover(bool hovered)
{
    m_window->isInUndoBtn(hovered);
}

void MainWindowToolBarHost::toolChanged(const QString &tool)
{
    m_window->changeShotToolEvent(tool);
}

void MainWindowToolBarHost::optionMenuShown()
{
    m_window->handleOptionMenuShown();
}

void MainWindowToolBarHost::keyboardToggled(bool enabled)
{
    m_window->changeKeyBoardShowEvent(enabled);
}

void MainWindowToolBarHost::mouseToggled(bool enabled)
{
    m_window->changeMouseShowEvent(enabled);
}

void MainWindowToolBarHost::cameraToggled(bool enabled)
{
    m_window->changeCameraSelectEvent(enabled);
}

void MainWindowToolBarHost::aiAssistantSelected(int function)
{
    m_window->onAiAssistantSelected(function);
}

void MainWindowToolBarHost::toolBarPointRequested()
{
    m_window->getToolBarPoint();
}

void MainWindowToolBarHost::switchToShotUI()
{
    m_window->onTreelandSwitchToShotUI();
}

void MainWindowToolBarHost::switchToRecordUI()
{
    m_window->onTreelandSwitchToRecordUI();
}

QPoint MainWindowToolBarHost::sideBarStartPressPoint() const
{
    m_window->getSideBarStartPressPoint();
    return QPoint();
}

QPoint MainWindowToolBarHost::toolBarStartPressPoint() const
{
    m_window->getToolBarStartPressPoint();
    return QPoint();
}

bool MainWindowToolBarHost::moveToolBars(const QPoint &start, const QPoint &delta)
{
    m_window->moveToolBars(start, delta);
    return true;
}
