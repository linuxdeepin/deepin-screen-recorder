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

#ifndef COLORTOOLWIDGET_H
#define COLORTOOLWIDGET_H

#include "toolbutton.h"

#include <DStackedWidget>
#include <DLabel>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class ColorToolWidget : public DLabel
{
    Q_OBJECT
public:
    explicit ColorToolWidget(DWidget *parent = nullptr);
    ~ColorToolWidget();

    void initWidget();
    //录屏截屏控件按钮初始化
    void initColorLabel();

signals:
    void colorChecked(QString colorType);

public slots:
    void setFunction(const QString &func);

private:
    QVBoxLayout *m_baseLayout;
    bool m_isChecked;
    QString m_function;
    ToolButton *m_redBtn;
    ToolButton *m_yellowBtn;
    ToolButton *m_blueBtn;
    ToolButton *m_greenBtn;
};

#endif // COLORTOOLWIDGET_H
