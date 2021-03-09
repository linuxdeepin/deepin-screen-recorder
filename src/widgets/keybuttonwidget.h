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

#ifndef KEYBUTTONWIDGET_H
#define KEYBUTTONWIDGET_H

#include <DLabel>
#include <DBlurEffectWidget>
#include <DFontSizeManager>
#include <DWidget>

#include <QObject>
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QFont>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class KeyButtonWidget : public DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit KeyButtonWidget(DWidget *parent = nullptr);
    ~KeyButtonWidget();

    //设置键盘按钮上的文字内容
    void setKeyLabelWord(const QString &keyWord);
signals:

public slots:

private:
    //键盘内容
    DLabel *m_word;
    QString m_keyword;
};

#endif // KEYBUTTONWIDGET_H
