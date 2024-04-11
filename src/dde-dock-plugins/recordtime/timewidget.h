// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

#define RECORDER_TIME_LEVEL_ICON_SIZE 23
#define RECORDER_TIME_VERTICAL_ICON_SIZE 22
#define RECORDER_TIME_VERTICAL_ICON_SIZE_1070 16
#define RECORDER_TIME_LEVEL_SIZE "00:00:00"
#define RECORDER_TIME_VERTICAL_SIZE "0000"
#define RECORDER_TIME_FONT DFontSizeManager::instance()->t8()
#define RECORDER_ICON_TOP_BOTTOM_X 8
#define RECORDER_TEXT_TOP_BOTTOM_X 10
#define RECORDER_TIME_WIDGET_MAXHEIGHT 40
#define RECORDER_TIME_WIDGET_MAXWIDTH 120

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

    /**
     * @brief 是否是wayland协议
     * @return
     */
    bool isWaylandProtocol();
    
protected:
    void showEvent(QShowEvent *) override;
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    /**
     * @brief 创建缓存文件，只有wayland模式下的mips或部分arm架构适用
     */
    bool createCacheFile();
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
    int m_systemVersion;

    /**
     * @brief m_lightIcon1070 1070下录屏计时图标icon
     */
    QIcon m_lightIcon1070;
    /**
     * @brief m_shadeIcon1070 1070下录屏计时图标icon
     */
    QIcon m_shadeIcon1070;
};

#endif // TIMEWIDGET_H
