/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Hou Lei <houlei@uniontech.com>
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

#ifndef MAINTOOLWIDGET_H
#define MAINTOOLWIDGET_H

#include "toolbutton.h"
#include "filter.h"

#include <DStackedWidget>
#include <DLabel>
#include <DWidget>

#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class MainToolWidget : public DStackedWidget
{
    Q_OBJECT
public:
    explicit MainToolWidget(DWidget *parent = nullptr);
    ~MainToolWidget();

    void initWidget();
    //录屏截屏控件按钮初始化
    void initMainLabel();
    void installTipHint(QWidget *w, const QString &hintstr);
    void installHint(QWidget *w, QWidget *hint);

signals:
    /**
     * @brief 发射截图功能或录屏功能被点击的信号
     * @param checked : 点击与否
     * @param type : "record" or "shot"
     */
    void buttonChecked(bool checked, QString type);
public slots:
//    void switchContent(QString shapeType);
    void setRecordButtonOut();
    void setRecordLauchMode(const unsigned int funType);


private:
    QHBoxLayout *m_baseLayout;
    DLabel *m_mainTool;
    bool m_isChecked;
    ToolButton *m_recordBtn;
    ToolButton *m_shotBtn;
    HintFilter *hintFilter = nullptr;
};

#endif // MAINTOOLWIDGET_H
