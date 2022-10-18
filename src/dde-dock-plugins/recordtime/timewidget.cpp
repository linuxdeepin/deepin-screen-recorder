// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "timewidget.h"
#include "dde-dock/constants.h"

#include <DGuiApplicationHelper>
#include <DStyle>

#include <QApplication>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QPixmap>
#include <QThread>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QPainterPath>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

TimeWidget::TimeWidget(DWidget *parent):
    DWidget(parent),
    m_timer(nullptr),
    m_dockInter(nullptr),
    m_lightIcon(nullptr),
    m_shadeIcon(nullptr),
    m_currentIcon(nullptr),
    m_bRefresh(true),
    m_position(-1),
    m_hover(false),
    m_pressed(false)
{
    QFontMetrics fm(RECORDER_TIME_FONT);
    m_showTimeStr = QString("00:00:00");
    m_textSize = fm.boundingRect("00:00:00 ").size();
    m_timer = new QTimer(this);
    m_dockInter = new DBusDock("com.deepin.dde.daemon.Dock", "/com/deepin/dde/daemon/Dock", QDBusConnection::sessionBus(), this);
    connect(m_dockInter, &DBusDock::PositionChanged, this, &TimeWidget::onPositionChanged);
    m_position = m_dockInter->position();
    m_lightIcon = new QIcon(":/res/light.svg");
    m_shadeIcon = new QIcon(":/res/shade.svg");
    m_currentIcon = m_lightIcon;
    //this->setAttribute(Qt::WA_StyledBackground,true);
    //this->setStyleSheet("background-color: rgb(255,255, 0)");
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
}

TimeWidget::~TimeWidget()
{
    if (nullptr != m_lightIcon) {
        delete m_lightIcon;
        m_lightIcon = nullptr;
    }
    if (nullptr != m_shadeIcon) {
        delete m_shadeIcon;
        m_shadeIcon = nullptr;
    }
    if (nullptr != m_timer) {
        m_timer->deleteLater();
        m_timer = nullptr;
    }
    if (nullptr != m_dockInter) {
        m_dockInter->deleteLater();
        m_dockInter = nullptr;
    }
}

bool TimeWidget::enabled()
{
    return isEnabled();
}

QSize TimeWidget::sizeHint() const
{
    QFontMetrics fm(RECORDER_TIME_FONT);
    int width = -1;
    int height = -1;
    if (position::top == m_position || position::bottom == m_position) {
        width = fm.boundingRect(RECORDER_TIME_LEVEL_SIZE).size().width() + 5 + RECORDER_TIME_LEVEL_ICON_SIZE + RECORDER_TEXT_TOP_BOTTOM_X;
        height = RECORDER_TIME_LEVEL_ICON_SIZE;
    } else if (position::left == m_position || position::right == m_position) {
        width = RECORDER_TIME_LEVEL_ICON_SIZE + RECORDER_TEXT_TOP_BOTTOM_X;
        height = RECORDER_TIME_VERTICAL_ICON_SIZE;
    }
    return QSize(width, this->geometry().height());
}

void TimeWidget::onTimeout()
{
    if (m_bRefresh) {
        if (m_currentIcon == m_lightIcon)
            m_currentIcon = m_shadeIcon;
        else
            m_currentIcon = m_lightIcon;
    }
    m_bRefresh = !m_bRefresh;
    QTime showTime(0, 0, 0);
    int time = m_baseTime.secsTo(QTime::currentTime());
    showTime = showTime.addSecs(time);
    m_showTimeStr = showTime.toString("hh:mm:ss");
    update();
}

void TimeWidget::onPositionChanged(int value)
{
    m_position = value;
}

void TimeWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    if (rect().height() > PLUGIN_BACKGROUND_MIN_SIZE) {
        QColor color;
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            color = Qt::black;
            painter.setOpacity(0.5);

            if (m_hover) {
                painter.setOpacity(0.6);
            }

            if (m_pressed) {
                painter.setOpacity(0.3);
            }
        } else {
            color = Qt::white;
            painter.setOpacity(0.1);

            if (m_hover) {
                painter.setOpacity(0.2);
            }

            if (m_pressed) {
                painter.setOpacity(0.05);
            }
        }
        painter.setPen(Qt::white);
        painter.setRenderHint(QPainter::Antialiasing, true);
        DStyleHelper dstyle(style());
        const int radius = dstyle.pixelMetric(DStyle::PM_FrameRadius);
        QPainterPath path;
        if (position::top == m_position || position::bottom == m_position) {
            QRect rc(0, 0, rect().width(), rect().height());
            rc.moveTo(rect().center() - rc.center());
            path.addRoundedRect(rc, radius, radius);

        } else if (position::right == m_position || position::left == m_position) {
            if (rect().width() > RECORDER_TIME_LEVEL_ICON_SIZE) {
                int minSize = std::min(width(), height());
                QRect rc(0, 0, minSize, minSize);
                rc.moveTo(rect().center() - rc.center());
                path.addRoundedRect(rc, radius, radius);
            } else {
                painter.setPen(Qt::black);
            }
        }
        painter.fillPath(path, color);
    } else {
        painter.setPen(Qt::black);
    }
    painter.setOpacity(1);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
    const auto ratio = devicePixelRatioF();
    //判断任务栏在屏幕上的位置,上下左右
    if (position::top == m_position || position::bottom == m_position) {
        m_pixmap = QIcon::fromTheme(QString("recordertime"), *m_currentIcon).pixmap(QSize(RECORDER_TIME_LEVEL_ICON_SIZE, RECORDER_TIME_LEVEL_ICON_SIZE));
        //m_pixmap.setDevicePixelRatio(ratio);
        const QRectF &rf = QRectF(rect());
        const QRectF &prf = QRectF(m_pixmap.rect());
        QPointF pf = rf.center() - prf.center();
        //绘制录像小图标
        painter.drawPixmap(5, static_cast<int>(pf.y() + 1), m_pixmap);
        QFont font = RECORDER_TIME_FONT;
        painter.setFont(font);
        QFontMetrics fm(font);
//        painter.drawText(m_pixmap.width() * static_cast<int>(ratio) + RECORDER_TEXT_TOP_BOTTOM_X + RECORDER_ICON_TOP_BOTTOM_X, rect().y(), rect().width(), rect().height(), Qt::AlignLeft | Qt::AlignVCenter, m_showTimeStr);
        int tx = static_cast<int>(m_pixmap.width()) + RECORDER_TEXT_TOP_BOTTOM_X;
        int ty = rect().y();
        int twidth = rect().width();
        int theight = rect().height();
        //绘制时间
        painter.drawText(tx, ty, twidth, theight, Qt::AlignLeft | Qt::AlignVCenter, m_showTimeStr);
    } else if (position::right == m_position || position::left == m_position) {
        m_pixmap = QIcon::fromTheme(QString("recordertime"), *m_currentIcon).pixmap(QSize(RECORDER_TIME_VERTICAL_ICON_SIZE, RECORDER_TIME_VERTICAL_ICON_SIZE) * ratio);
        m_pixmap.setDevicePixelRatio(ratio);
        const QRectF &rf = QRectF(rect());
        const QRectF &rfp = QRectF(m_pixmap.rect());
        painter.drawPixmap(rf.center() - rfp.center() / m_pixmap.devicePixelRatioF(), m_pixmap);
    }
    QWidget::paintEvent(e);
}

void TimeWidget::mousePressEvent(QMouseEvent *e)
{
    qDebug() << "Click the taskbar plugin! To start!";
    m_pressed = true;
    int width = 0;
    //任务栏的位置在桌面顶部和底部时才会显示录屏时间
    if (position::top == m_position || position::bottom == m_position) {
        width = rect().width();
    } else {
        width = m_pixmap.width();
    }
    bool flag = true;
    if (e->pos().x() > 0 && e->pos().x() < width) {
#if  defined (__mips__) || defined (__sw_64__) || defined (__loongarch_64__) || defined (__aarch64__) || defined (__loongarch__)
        if (isWaylandProtocol()) {
            flag = false;
            createCacheFile();
        }
#endif
        if (flag) {
            qDebug() << "Click the taskbar plugin! Dbus call stop recording screen!";
            QDBusInterface notification(QString::fromUtf8("com.deepin.ScreenRecorder"),
                                        QString::fromUtf8("/com/deepin/ScreenRecorder"),
                                        QString::fromUtf8("com.deepin.ScreenRecorder"),
                                        QDBusConnection::sessionBus());
            notification.asyncCall("stopRecord");
            //        QDBusMessage message = notification.call("stopRecord"); //会阻塞任务其他按钮的执行
        }
    }
    update();
    QWidget::mousePressEvent(e);
    qDebug() << "Click the taskbar plugin! The end!";
}
//创建缓存文件，只有wayland模式下的mips或部分arm架构适用
void TimeWidget::createCacheFile()
{
    qDebug() << "createCacheFile start!";
    QString userName = QDir::homePath().section("/", -1, -1);
    std::string path = ("/home/" + userName + "/.cache/deepin/deepin-screen-recorder/").toStdString();
    QDir tdir(path.c_str());
    //判断文件夹路径是否存在
    if (!tdir.exists()) {
        tdir.mkpath(path.c_str());
    }
    path += "stopRecord.txt";
    //判断文件是否存在，若存在则先删除文件
    QFile mFile(path.c_str());
    if (mFile.exists()) {
        remove(path.c_str());
    }
    //打开文件
    int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        qDebug() << "open file fail!" << strerror(errno);
        return;
    }
    //文件加锁
    int flock = lockf(fd, F_TLOCK, 0);
    if (flock == -1) {
        qDebug() << "lock file fail!" << strerror(errno);
        return;
    }
    ssize_t ret = -1;
    //文件内容为1，读取文件时会停止录屏
    char wBuffer[2] = {"1"};
    //写文件
    ret = write(fd, wBuffer, 2);
    if (ret < 0) {
        qDebug() << "write file fail!";
        return ;
    }
    flock = lockf(fd, F_ULOCK, 0);
    ::close(fd);
    qDebug() << "createCacheFile end!";

}

void TimeWidget::mouseReleaseEvent(QMouseEvent *e)
{
    m_pressed = false;
    m_hover = false;
    update();
    QWidget::mouseReleaseEvent(e);
}

void TimeWidget::mouseMoveEvent(QMouseEvent *e)
{
    m_hover = true;
    update();
    QWidget::mouseMoveEvent(e);
}

void TimeWidget::leaveEvent(QEvent *e)
{
    m_hover = false;
    m_pressed = false;
    update();
    QWidget::leaveEvent(e);
}

void TimeWidget::start()
{
    m_showTimeStr = QString("00:00:00");
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_baseTime = QTime::currentTime();
    m_timer->start(400);
}

void TimeWidget::stop()
{
    disconnect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}
//判断是否是wayland协议
bool TimeWidget::isWaylandProtocol()
{
    QProcessEnvironment e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    return XDG_SESSION_TYPE == QLatin1String("wayland") ||  WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive);
}
