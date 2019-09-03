/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     Zheng Youge<youge.zheng@deepin.com>
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

#include <DLabel>
#include <QPainter>
#include <DBlurEffectWidget>
#include <QEvent>
#include <QDebug>

#include "colortoolwidget.h"
#include "shottoolwidget.h"
DWIDGET_USE_NAMESPACE

class SideBarWidget : public DBlurEffectWidget
{
    Q_OBJECT
public:
    SideBarWidget(QWidget *parent = nullptr);
    ~SideBarWidget() Q_DECL_OVERRIDE;

    void changeShotToolWidget(const QString &func);
signals:
    void changeArrowAndLineEvent(int line);
protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
private:
    QLabel *m_hSeparatorLine;
    ColorToolWidget *m_colorTool;
    ShotToolWidget *m_shotTool;

    bool  m_expanded;
};

class SideBar : public DLabel
{
    Q_OBJECT
public:
    explicit SideBar(QWidget *parent = nullptr);
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
    bool isButtonChecked();
    void setExpand(bool expand, QString shapeType);
    void showAt(QPoint pos);

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
    SideBarWidget *m_sidebarWidget;
    bool m_expanded;
};

#endif // SIDEBAR_H
