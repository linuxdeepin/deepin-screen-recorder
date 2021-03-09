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

#ifndef TIMEWIDGET_H
#define TIMEWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QTime>
#include <QIcon>
#include <DWidget>
#include <com_deepin_dde_daemon_dock.h>
#include <DFontSizeManager>
#include <QBoxLayout>
#include <QLabel>

#define RECORDER_TIME_LEVEL_ICON_SIZE 22
#define RECORDER_TIME_VERTICAL_ICON_SIZE 22
#define RECORDER_TIME_LEVEL_SIZE "00000 00:00:00"
#define RECORDER_TIME_VERTICAL_SIZE "0000"
#define RECORDER_TIME_FONT DFontSizeManager::instance()->t8()
#define RECORDER_ICON_TOP_BOTTOM_X 8
#define RECORDER_TEXT_TOP_BOTTOM_X 3

DWIDGET_USE_NAMESPACE
using DBusDock = com::deepin::dde::daemon::Dock;

class TimeWidget : public DWidget
{
    Q_OBJECT

    enum position {
        top = 0,
        right,
        bottom,
        left
    };

public:
    explicit TimeWidget(DWidget *parent = nullptr);
    ~TimeWidget();
    bool enabled();

    /**
     * @brief start:开始计时
     */
    void start();

    /**
     * @brief stop:停止计时
     */
    void stop();

    /**
     * @brief sizeHint:返回控件大小
     * @return
     */
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;

private slots:
    /**
     * @brief onTimeout:更新数据
     */
    void onTimeout();

    /**
     * @brief onPositionChanged:dde-dock位置变化通知
     * @param value
     */
    void onPositionChanged(int value);

private:
    QTimer *m_timer;
    DBusDock *m_dockInter;
    QIcon *m_lightIcon;
    QIcon *m_shadeIcon;
    QIcon *m_currentIcon;
    QPixmap m_pixmap;
    QSize m_textSize;
    QTime m_baseTime;
    QString m_showTimeStr;
    bool m_bRefresh;
    int m_position;
    QBoxLayout *centralLayout;
    bool m_hover;
    bool m_pressed;
};

#endif // TIMEWIDGET_H
