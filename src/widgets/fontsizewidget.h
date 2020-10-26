/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Maintainer: Peng Hui<penghui@deepin.com>
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

#ifndef FONTSIZEWIDGET_H
#define FONTSIZEWIDGET_H

#include <DLabel>
#include <DWidget>
#include <DLineEdit>
#include <DPushButton>

DWIDGET_USE_NAMESPACE

class Separator : public DLabel
{
    Q_OBJECT
public:
    explicit Separator(DWidget *parent);
    ~Separator();
};

class FontSizeWidget : public DLabel
{
    Q_OBJECT
public:
    explicit FontSizeWidget(DWidget *parent = 0);
    ~FontSizeWidget();

    void initWidget();
    void adjustFontSize(bool add);
    void setFontSize(int fontSize);

signals:
    void fontSizeChanged(int fontSize);

private:
    DLineEdit *m_fontSizeEdit;
    DPushButton *m_addSizeBtn;
    DPushButton *m_reduceSizeBtn;
    int m_fontSize;
};
#endif // FONTSIZEWIDGET_H
