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

#ifndef TOPTIPS_H
#define TOPTIPS_H

#include <DLabel>
#include <DWidget>
#include <QLabel>

DWIDGET_USE_NAMESPACE

class TopTips : public QLabel
{
    Q_OBJECT
public:
    explicit TopTips(DWidget *parent = nullptr);
    ~TopTips() override;
    void setRecorderTipsInfo(const bool showState);
public slots:
    void setContent(const QSize &rect);
    void updateTips(QPoint pos, const QSize &rect);

protected:
    void mousePressEvent(QMouseEvent *ev) override;
    bool m_showRecorderTips = false;
    int m_width = 0;
    int m_height = 0;
};
#endif // TOPTIPS_H
