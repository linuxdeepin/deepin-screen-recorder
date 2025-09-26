// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SAVEBUTTON_H
#define SAVEBUTTON_H

#include "toolbutton.h"
#include <QIcon>
#include <QHBoxLayout>

class QMenu;

class SaveButton : public ToolButton
{
    Q_OBJECT
public:
    explicit SaveButton(QWidget *parent = nullptr);
    ~SaveButton();

    // 设置外部菜单
    void setOptionsMenu(QMenu *menu);
    void setSaveIcon(const QIcon &icon);
    void setListIcon(const QIcon &icon);

signals:
    void saveAction();
    void expandSaveOption(bool expand);
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void onMenuAboutToHide();

private:
    QMenu *m_optionsMenu = nullptr;  // 外部传入的菜单
    QIcon m_saveIcon;
    QIcon m_listIcon;
    
    bool m_hoverFlag = false;
    bool m_saveClicked = false;
    bool m_listClicked = false;
    int currentMouseX = 0;  
    
    static constexpr int kSaveButtonWidth = 46;
    static constexpr int kSaveButtonHeight = 36;
    static constexpr int kSaveAreaWidth = 30;  
    static constexpr int kListAreaWidth = 16;  
    static constexpr int kCornerRadius = 8;    
};

#endif // SAVEBUTTON_H
