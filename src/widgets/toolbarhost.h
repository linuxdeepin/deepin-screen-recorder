// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLBARHOST_H
#define TOOLBARHOST_H

#include <QObject>
#include <QPoint>

class QWidget;
class MainWindow;

/**
 * Host contract shared by the legacy screenshot toolbar and alternative
 * screenshot frontends.
 *
 * The widgets keep their existing visual implementation. The host only owns
 * application actions, which prevents ToolBar/SubToolWidget/SideBar from
 * depending directly on MainWindow.
 */
class ToolBarHost : public QObject
{
    Q_OBJECT
public:
    explicit ToolBarHost(QWidget *anchorWidget, QObject *parent = nullptr)
        : QObject(parent)
        , m_anchorWidget(anchorWidget)
    {
    }

    QWidget *anchorWidget() const
    {
        return m_anchorWidget;
    }

    void setAnchorWidget(QWidget *anchorWidget)
    {
        m_anchorWidget = anchorWidget;
    }

    virtual void confirm() = 0;
    virtual void close(bool hideToolBar) = 0;
    virtual void save() = 0;
    virtual void undo() = 0;
    virtual void undoHover(bool hovered) = 0;
    virtual void toolChanged(const QString &tool) = 0;
    virtual void optionMenuShown() = 0;
    virtual void keyboardToggled(bool enabled) = 0;
    virtual void mouseToggled(bool enabled) = 0;
    virtual void cameraToggled(bool enabled) = 0;
    virtual void aiAssistantSelected(int function) = 0;
    virtual void toolBarPointRequested() = 0;
    virtual void switchToShotUI() = 0;
    virtual void switchToRecordUI() = 0;
    virtual QPoint sideBarStartPressPoint() const = 0;
    virtual QPoint toolBarStartPressPoint() const = 0;
    virtual bool moveToolBars(const QPoint &start, const QPoint &delta) = 0;

signals:
    void microphoneEnabled(bool enabled);
    void undoAvailabilityChanged(bool enabled);

private:
    QWidget *m_anchorWidget = nullptr;
};

/**
 * Adapter preserving the existing MainWindow behaviour for the original
 * desktop screenshot flow.
 */
class MainWindowToolBarHost final : public ToolBarHost
{
    Q_OBJECT
public:
    explicit MainWindowToolBarHost(MainWindow *window, QWidget *anchor);

    void confirm() override;
    void close(bool hideToolBar) override;
    void save() override;
    void undo() override;
    void undoHover(bool hovered) override;
    void toolChanged(const QString &tool) override;
    void optionMenuShown() override;
    void keyboardToggled(bool enabled) override;
    void mouseToggled(bool enabled) override;
    void cameraToggled(bool enabled) override;
    void aiAssistantSelected(int function) override;
    void toolBarPointRequested() override;
    void switchToShotUI() override;
    void switchToRecordUI() override;
    QPoint sideBarStartPressPoint() const override;
    QPoint toolBarStartPressPoint() const override;
    bool moveToolBars(const QPoint &start, const QPoint &delta) override;

private:
    MainWindow *m_window = nullptr;
};

#endif // TOOLBARHOST_H
