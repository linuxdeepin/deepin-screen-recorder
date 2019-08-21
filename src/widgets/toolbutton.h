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

#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <DPushButton>
#include <QApplication>
#include <QPalette>

DWIDGET_USE_NAMESPACE

class ToolButton : public DPushButton
{
    Q_OBJECT
public:
    ToolButton(QWidget *parent = 0)
    {
        Q_UNUSED(parent);
        setCheckable(true);
        m_tips = "";
//        connect(this, SIGNAL(toggled(bool)), this, SLOT(changeButtonStatus(bool)));
    }
    ~ToolButton() {}

public slots:
    void setTips(QString tips)
    {
        m_tips = tips;
    }

    QString getTips()
    {
        return m_tips;
    }

    void changeButtonStatus(bool checked)
    {
        QPalette pa;
        if (checked == true) {
            pa = this->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, Qt::black);
            pa.setColor(QPalette::Light, Qt::black);
            this->setPalette(pa);
        }

        else {
            pa = this->palette();
            pa.setColor(QPalette::ButtonText, pa.buttonText().color());
            pa.setColor(QPalette::Dark, pa.dark().color());
            pa.setColor(QPalette::Light, pa.light().color());
            this->setPalette(pa);
        }
    }

signals:
    void onEnter();
    void onExist();

protected:
    void enterEvent(QEvent *e) override
    {
        emit onEnter();
        QPushButton::enterEvent(e);
    }

    void leaveEvent(QEvent *e) override
    {
        emit onExist();
        QPushButton::leaveEvent(e);
    }

private:
    QString m_tips;

};
#endif // TOOLBUTTON_H
