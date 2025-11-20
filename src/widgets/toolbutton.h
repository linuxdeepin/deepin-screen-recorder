// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <DToolButton>
#include <DPalette>

#include <QApplication>
#include <QIcon>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class ToolButton : public DToolButton
{
    Q_OBJECT
public:
    explicit ToolButton(QWidget *parent = nullptr);
    ~ToolButton();
    
    void setHoverState(const bool hasHover);
    void setOptionButtonFlag(const bool flag);
    void setUndoButtonFlag(const bool flag);
    
    // 红点角标控制
    void setShowRedDot(bool show) { m_showRedDot = show; update(); }
    
    // Icon 角标控制
    void setBadgeIcon(const QIcon &icon);
    void setBadgeIcon(const QString &iconPath);
    void setBadgeSize(const QSize &size) { m_badgeSize = size; update(); }
    void clearBadge() { m_badgeIcon = QIcon(); update(); }
protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
signals:
    /**
     * @brief isInButton 是否在button内部？
     * @param isUndo
     */
    void isInUndoBtn(bool isInUndo);
private:
    bool m_hasHoverState;
    bool m_isOptionButton = false;
    bool m_isUndoButton = false;
    bool m_isMousePress = false;
    QCursor *m_lastCursorShape = nullptr;
    bool m_menuActive = false;
    
    // 红点角标
    bool m_showRedDot = false;
    
    // Icon 角标
    QIcon m_badgeIcon;
    QSize m_badgeSize = QSize(10, 7);
    
    // 计算图标区域
    QRect getIconRect(const QStyleOptionToolButton &opt) const;
    // 绘制红点角标
    void drawRedDot(QPainter &painter, const QStyleOptionToolButton &opt, const QRect &iconRect);
    // 绘制Icon角标
    void drawBadge(QPainter &painter, const QStyleOptionToolButton &opt, const QRect &iconRect);
};
#endif // TOOLBUTTON_H
