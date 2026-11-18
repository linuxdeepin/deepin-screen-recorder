// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../../src/widgets/toolbarhost.h"

// Lightweight host used by widget tests. ToolBar, SubToolWidget and SideBar no
// longer depend on MainWindow directly, so tests should not need a fully wired
// MainWindow just to satisfy their constructors.
class TestToolBarHost final : public ToolBarHost
{
public:
    explicit TestToolBarHost(QWidget *anchorWidget = nullptr)
        : ToolBarHost(anchorWidget)
    {
    }

    void confirm() override {}
    void close(bool) override {}
    void save() override {}
    void undo() override {}
    void undoHover(bool) override {}
    void toolChanged(const QString &) override {}
    void optionMenuShown() override {}
    void keyboardToggled(bool) override {}
    void mouseToggled(bool) override {}
    void cameraToggled(bool) override {}
    void aiAssistantSelected(int) override {}
    void toolBarPointRequested() override {}
    void switchToShotUI() override {}
    void switchToRecordUI() override {}
    QPoint sideBarStartPressPoint() const override { return QPoint(); }
    QPoint toolBarStartPressPoint() const override { return QPoint(); }
    bool moveToolBars(const QPoint &, const QPoint &) override { return true; }
};
