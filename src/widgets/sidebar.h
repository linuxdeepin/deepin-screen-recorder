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

#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "colortoolwidget.h"
#include "shottoolwidget.h"
#include "toolbutton.h"

#include <DLabel>
#include <DBlurEffectWidget>
#include <DImageButton>
#include <DFloatingWidget>

#include <QPainter>
#include <QEvent>
#include <QDebug>

DWIDGET_USE_NAMESPACE

class SideBarWidget : public DFloatingWidget
{
    Q_OBJECT
public:
    explicit SideBarWidget(DWidget *parent = nullptr);
    ~SideBarWidget() Q_DECL_OVERRIDE;

    void changeShotToolWidget(const QString &func);
signals:
    void changeArrowAndLineEvent(int line);
    void closeSideBar();
protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private:
    DLabel *m_hSeparatorLine;
    ColorToolWidget *m_colorTool;
    ShotToolWidget *m_shotTool;
    DImageButton *m_closeButton;
    bool  m_expanded;
};

class SideBar : public DLabel
{
    Q_OBJECT
public:
    explicit SideBar(DWidget *parent = nullptr);
    ~SideBar() Q_DECL_OVERRIDE;

    void changeShotToolFunc(const QString &func);
signals:
    void heightChanged();
    void buttonChecked(QString shape);
    void updateColor(QColor color);
    void requestSaveScreenshot();
    void shapePressed(QString tool);
    void closed();
    void changeArrowAndLineToMain(int line);
public slots:
    //bool isButtonChecked();
    //void setExpand(bool expand, QString shapeType);
    void showAt(QPoint pos);
    void initSideBar();
    //void setColorFunc(const QString &func);

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
    SideBarWidget *m_sidebarWidget;

    bool m_expanded;
};

#endif // SIDEBAR_H
