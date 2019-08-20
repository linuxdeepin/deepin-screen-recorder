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
#ifndef MAINTOOLWIDGET_H
#define MAINTOOLWIDGET_H

#include <QStackedWidget>
#include <QHBoxLayout>
#include <QLabel>

class MainToolWidget : public QLabel
{
    Q_OBJECT
public:
    explicit MainToolWidget(QWidget* parent = nullptr);
    ~MainToolWidget();

    void initWidget();
    //录屏截屏控件按钮初始化
    void initMainLabel();

signals:
    void buttonChecked(bool checked, QString type);
public slots:
//    void switchContent(QString shapeType);

private:
    QHBoxLayout* m_baseLayout;
    bool m_isChecked;
};

#endif // MAINTOOLWIDGET_H
