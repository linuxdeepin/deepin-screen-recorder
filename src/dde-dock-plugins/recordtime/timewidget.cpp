// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "timewidget.h"
#include "dde-dock/constants.h"
#include "../../utils/log.h"

#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DStyle>
#include <DSysInfo>

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

DCORE_USE_NAMESPACE

#define RECORDER_TIME_LEVEL_ICON_SIZE 23
#define RECORDER_TIME_VERTICAL_ICON_SIZE 16
#define RECORDER_TIME_FONT DFontSizeManager::instance()->t8()
#define RECORDER_PADDING 1
#define RECORDER_TIME_STARTCONFIG "CurrentStartTime"
#define RECORDER_TIME_STARTCOUNTCONFIG "CurrentStartCount"

TimeWidget::TimeWidget(DWidget *parent):
    DWidget(parent),
    m_timer(nullptr),
    m_dockInter(nullptr),
    m_lightIcon(nullptr),
    m_shadeIcon(nullptr),
    m_currentIcon(nullptr),
    m_bRefresh(true),
    m_position(Dock::Position::Bottom),
    m_hover(false),
    m_pressed(false),
    m_systemVersion(0),
    m_timerCount(0),
    m_setting(nullptr)
{
    qCDebug(dsrApp) << "TimeWidget constructor called.";
    setContentsMargins(0, 0, 0, 0);

    auto *layout = new QHBoxLayout(this);
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    m_iconLabel = new QLabel(this);
    m_textLabel = new QLabel(this);
    layout->addWidget(m_iconLabel);
    layout->addWidget(m_textLabel);

    m_textLabel->setFont(RECORDER_TIME_FONT);
    m_textLabel->setText("00:00:00");

    auto updatePalette = [this](){
        qCDebug(dsrApp) << "Updating palette based on theme type:" << DGuiApplicationHelper::instance()->themeType();
        QPalette textPalette = m_textLabel->palette();
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            textPalette.setColor(QPalette::WindowText, Qt::black);
            qCDebug(dsrApp) << "Theme is LightType, setting text color to black.";
        }else{
            textPalette.setColor(QPalette::WindowText, Qt::white);
            qCDebug(dsrApp) << "Theme is not LightType, setting text color to white.";
        }
        m_textLabel->setPalette(textPalette);
    };
    updatePalette();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, updatePalette);

    m_textLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_timer = new QTimer(this);
    m_dockInter = new timewidget_interface("org.deepin.dde.daemon.Dock1",
                                         "/org/deepin/dde/daemon/Dock1",
                                         QDBusConnection::sessionBus(),
                                         this);
    connect(m_dockInter, SIGNAL(PositionChanged(int)),
            this, SLOT(onPositionChanged(int)));
    
    m_position = m_dockInter->position();
    m_lightIcon = new QIcon(":/res/1070/light.svg");
    m_shadeIcon = new QIcon(":/res/1070/shade.svg");

    m_currentIcon = m_lightIcon;

    updateIcon();
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    m_setting = new QSettings("deepin/deepin-screen-recorder", "recordtime", this);
    qCDebug(dsrApp) << "TimeWidget constructor finished.";
}

TimeWidget::~TimeWidget()
{
    qCDebug(dsrApp) << "TimeWidget destructor called.";
    if (nullptr != m_lightIcon) {
        qCDebug(dsrApp) << "Deleting m_lightIcon.";
        delete m_lightIcon;
        m_lightIcon = nullptr;
    }
    if (nullptr != m_shadeIcon) {
        qCDebug(dsrApp) << "Deleting m_shadeIcon.";
        delete m_shadeIcon;
        m_shadeIcon = nullptr;
    }
    if (nullptr != m_timer) {
        qCDebug(dsrApp) << "Deleting m_timer.";
        m_timer->deleteLater();
        m_timer = nullptr;
    }
    if (nullptr != m_dockInter) {
        qCDebug(dsrApp) << "Deleting m_dockInter.";
        m_dockInter->deleteLater();
        m_dockInter = nullptr;
    }
    if (nullptr != m_setting){
        qCDebug(dsrApp) << "Deleting m_setting.";
        m_setting->deleteLater();
        m_setting = nullptr;
    }
    qCDebug(dsrApp) << "TimeWidget destructor finished.";
}

bool TimeWidget::enabled()
{
    qCDebug(dsrApp) << "enabled method called.";
    return isEnabled();
}

void TimeWidget::onTimeout()
{
    qCDebug(dsrApp) << "onTimeout method called.";
    m_timerCount++;
    if (m_bRefresh) {
        qCDebug(dsrApp) << "m_bRefresh is true, toggling current icon.";
        if (m_currentIcon == m_lightIcon) {
            m_currentIcon = m_shadeIcon;
            qCDebug(dsrApp) << "Switched icon to shade.";
        }
        else {
            m_currentIcon = m_lightIcon;
            qCDebug(dsrApp) << "Switched icon to light.";
        }
    } else {
        qCDebug(dsrApp) << "m_bRefresh is false, not toggling icon.";
    }
    m_bRefresh = !m_bRefresh;
    QTime showTime(0, 0, 0);
    showTime = showTime.addMSecs(m_timerCount * 400);
    m_setting->setValue(RECORDER_TIME_STARTCOUNTCONFIG, m_timerCount);
    m_textLabel->setText(showTime.toString("hh:mm:ss"));
    updateIcon();
    qCDebug(dsrApp) << "onTimeout method finished. Current time:" << showTime.toString("hh:mm:ss");
}

void TimeWidget::updateIcon() {
    qCDebug(dsrApp) << "updateIcon method called.";
    if (Dock::Position::Top == m_position || Dock::Position::Bottom == m_position) {
        m_pixmap = QIcon::fromTheme(QString("recordertime"), *m_currentIcon).pixmap(QSize(RECORDER_TIME_VERTICAL_ICON_SIZE, RECORDER_TIME_VERTICAL_ICON_SIZE));
        qCDebug(dsrApp) << "Position is Top or Bottom, setting vertical icon size.";
    } else {
        m_pixmap = QIcon::fromTheme(QString("recordertime"), *m_currentIcon).pixmap(QSize(RECORDER_TIME_VERTICAL_ICON_SIZE, RECORDER_TIME_VERTICAL_ICON_SIZE));
        qCDebug(dsrApp) << "Position is not Top or Bottom, setting vertical icon size.";
    }

    m_iconLabel->setPixmap(m_pixmap);
    qCDebug(dsrApp) << "updateIcon method finished.";
}

void TimeWidget::onPositionChanged(int value)
{
    qCDebug(dsrApp) << "onPositionChanged method called with value:" << value;
    qInfo() << "====================== onPositionChanged ====start=================";
    m_position = value;

    if (m_position == Dock::Position::Top || m_position == Dock::Position::Bottom) {
        m_textLabel->show();
        qCDebug(dsrApp) << "Position is Top or Bottom, showing text label.";
    } else {
        m_textLabel->hide();
        qCDebug(dsrApp) << "Position is not Top or Bottom, hiding text label.";
    }

    qInfo() << "====================== onPositionChanged ====end=================";
    qCDebug(dsrApp) << "onPositionChanged method finished.";
}

QSettings *TimeWidget::setting() const
{
    qCDebug(dsrApp) << "setting method called.";
    return m_setting;
}

void TimeWidget::clearSetting()
{
    qCDebug(dsrApp) << "clearSetting method called.";
    if (m_setting) {
        m_setting->setValue(RECORDER_TIME_STARTCONFIG, QTime(0,0,0));
        m_setting->setValue(RECORDER_TIME_STARTCOUNTCONFIG, 0);
        qCDebug(dsrApp) << "Settings cleared: RECORDER_TIME_STARTCONFIG and RECORDER_TIME_STARTCOUNTCONFIG.";
    } else {
        qCDebug(dsrApp) << "m_setting is null, cannot clear settings.";
    }
    qCDebug(dsrApp) << "clearSetting method finished.";
}

void TimeWidget::paintEvent(QPaintEvent *e)
{
    qCDebug(dsrApp) << "paintEvent method called.";
    //qInfo() << "====================== paintEvent ====start=================";
    QPainter painter(this);
    //qInfo() <<  ">>>>>>>>>> rect().width(): " << rect().width() << " , this->height(): " << this->height();

    if (rect().height() > PLUGIN_BACKGROUND_MIN_SIZE) {
        qCDebug(dsrApp) << "Height is greater than PLUGIN_BACKGROUND_MIN_SIZE.";
        QColor color;
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            color = Qt::black;
            painter.setOpacity(0);
            qCDebug(dsrApp) << "Theme is LightType, setting color to black and opacity to 0.";
            if (m_hover) {
                painter.setOpacity(0.6);
                qCDebug(dsrApp) << "Hover is true, setting opacity to 0.6.";
            }

            if (m_pressed) {
                painter.setOpacity(0.3);
                qCDebug(dsrApp) << "Pressed is true, setting opacity to 0.3.";
            }
        } else {
            color = Qt::white;
            painter.setOpacity(0);
            qCDebug(dsrApp) << "Theme is not LightType, setting color to white and opacity to 0.";
            if (m_hover) {
                painter.setOpacity(0.2);
                qCDebug(dsrApp) << "Hover is true, setting opacity to 0.2.";
            }

            if (m_pressed) {
                painter.setOpacity(0.05);
                qCDebug(dsrApp) << "Pressed is true, setting opacity to 0.05.";
            }
        }
        painter.setRenderHint(QPainter::Antialiasing, true);
        DStyleHelper dstyle(style());
        const int radius = dstyle.pixelMetric(DStyle::PM_FrameRadius);
        QPainterPath path;
        if ((Dock::Position::Top == m_position || Dock::Position::Bottom == m_position) ||
            ((Dock::Position::Right == m_position || Dock::Position::Left == m_position) && rect().width() > RECORDER_TIME_LEVEL_ICON_SIZE)) {
            QRect rc(RECORDER_PADDING, RECORDER_PADDING, rect().width() - RECORDER_PADDING * 2, rect().height() - RECORDER_PADDING * 2);
            path.addRoundedRect(rc, radius, radius);
            qCDebug(dsrApp) << "Position is Top/Bottom or Right/Left with sufficient width, adding rounded rect path.";
        } else {
            qCDebug(dsrApp) << "Conditions not met for adding rounded rect path.";
        }
        painter.fillPath(path, color);
    } else {
        qCDebug(dsrApp) << "Height is not greater than PLUGIN_BACKGROUND_MIN_SIZE.";
    }

    QWidget::paintEvent(e);
    qCDebug(dsrApp) << "paintEvent method finished.";
    //qInfo() << "====================== paintEvent ====end=================";
}

void TimeWidget::mousePressEvent(QMouseEvent *e)
{
    qCDebug(dsrApp) << "Click the taskbar plugin! To start!";
    m_pressed = true;
    repaint();
    QWidget::mousePressEvent(e);
    qCDebug(dsrApp) << "Click the taskbar plugin! The end!";
}
//创建缓存文件，只有wayland模式下的mips或部分arm架构适用
bool TimeWidget::createCacheFile()
{
    qCDebug(dsrApp) << "createCacheFile start!";
    QString userName = QDir::homePath().section("/", -1, -1);
    std::string path = ("/home/" + userName + "/.cache/deepin/deepin-screen-recorder/").toStdString();
    QDir tdir(path.c_str());
    //判断文件夹路径是否存在
    if (!tdir.exists()) {
        tdir.mkpath(path.c_str());
        qCDebug(dsrApp) << "Cache directory did not exist, created:" << QString::fromStdString(path);
    } else {
        qCDebug(dsrApp) << "Cache directory already exists:" << QString::fromStdString(path);
    }
    path += "stopRecord.txt";
    //判断文件是否存在，若存在则先删除文件
    QFile mFile(path.c_str());
    if (mFile.exists()) {
        qCDebug(dsrApp) << "Removing existing cache file";
        remove(path.c_str());
    } else {
        qCDebug(dsrApp) << "Cache file does not exist, no removal needed.";
    }
    //打开文件
    int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        qCCritical(dsrApp) << "Failed to open cache file:" << strerror(errno);
        return false;
    }
    //文件加锁
    int flock = lockf(fd, F_TLOCK, 0);
    if (flock == -1) {
        qCDebug(dsrApp) << "lock file fail!" << strerror(errno);
        ::close(fd);
        return false;
    }
    ssize_t ret = -1;
    //文件内容为1，读取文件时会停止录屏
    char wBuffer[2] = {"1"};
    //写文件
    ret = write(fd, wBuffer, 2);
    if (ret < 0) {
        qCCritical(dsrApp) << "Failed to write to cache file";
        lockf(fd, F_ULOCK, 0);
        ::close(fd);
        return false;
    }
    flock = lockf(fd, F_ULOCK, 0);
    if (flock == -1) {
        qCCritical(dsrApp) << "Failed to unlock cache file:" << strerror(errno);
        ::close(fd);
        return false;
    }
    ::close(fd);
    qCInfo(dsrApp) << "Cache file created successfully";
    return true;
}

void TimeWidget::mouseReleaseEvent(QMouseEvent *e)
{
    qCDebug(dsrApp) << "Mouse release event received";
    if(e->button() == Qt::LeftButton && m_pressed && m_hover){
        qCDebug(dsrApp) << "Left button released while pressed and hovered. Resetting flags.";
        m_pressed = false;
        m_hover = false;
        update();
        QWidget::mouseReleaseEvent(e);
        qCDebug(dsrApp) << "mouseReleaseEvent method finished (early exit).";
        return;
    }
    int width = rect().width();
    bool flag = true;
    if (e->pos().x() > 0 && e->pos().x() < width) {
        qCDebug(dsrApp) << "Mouse release position is within widget width.";
#if  defined (__mips__) || defined (__sw_64__) || defined (__loongarch_64__) || defined (__aarch64__) || defined (__loongarch__)
        if (isWaylandProtocol()) {
            qCDebug(dsrApp) << "Wayland protocol detected for specific architectures.";
            flag = false;
            if(!createCacheFile()){
                qCWarning(dsrApp) << "Failed to create cache file for stopping recording";
                flag = true;
            };
        }
#endif
        if (flag) {
            qCDebug(dsrApp) << "Click the taskbar plugin! Dbus call stop recording screen!";;
            QDBusInterface notification(QString::fromUtf8("com.deepin.ScreenRecorder"),
                                        QString::fromUtf8("/com/deepin/ScreenRecorder"),
                                        QString::fromUtf8("com.deepin.ScreenRecorder"),
                                        QDBusConnection::sessionBus());
            notification.asyncCall("stopRecord");
            //        QDBusMessage message = notification.call("stopRecord"); //会阻塞任务其他按钮的执行
        }
    }
    m_pressed = false;
    m_hover = false;
    repaint();
    QWidget::mouseReleaseEvent(e);
    qCDebug(dsrApp) << "Mouse release end!";

}

void TimeWidget::mouseMoveEvent(QMouseEvent *e)
{
    qCDebug(dsrApp) << "mouseMoveEvent method called.";
    m_hover = true;
    repaint();
    QWidget::mouseMoveEvent(e);
    qCDebug(dsrApp) << "mouseMoveEvent method finished.";
}

void TimeWidget::leaveEvent(QEvent *e)
{
    qCDebug(dsrApp) << "leaveEvent method called.";
    m_hover = false;
    m_pressed = false;
    repaint();
    QWidget::leaveEvent(e);
    qCDebug(dsrApp) << "leaveEvent method finished.";
}

void TimeWidget::start()
{
    qCDebug(dsrApp) << "start method called.";
    qCInfo(dsrApp) << "Starting time widget";
    if (m_setting->value(RECORDER_TIME_STARTCONFIG).toTime() == QTime(0, 0, 0)) {
        qCDebug(dsrApp) << "Initializing start time";
        m_setting->setValue(RECORDER_TIME_STARTCONFIG, QTime::currentTime());
        m_baseTime = QTime::currentTime();
    } else {
        qCDebug(dsrApp) << "Loading existing start time.";
        m_baseTime = m_setting->value(RECORDER_TIME_STARTCONFIG).toTime();
    }

    if (m_setting->value(RECORDER_TIME_STARTCOUNTCONFIG).toInt() == 0) {
        qCDebug(dsrApp) << "Initializing timer count";
        m_setting->setValue(RECORDER_TIME_STARTCOUNTCONFIG, 0);
        m_timerCount = 0;
    } else {
        qCDebug(dsrApp) << "Loading existing timer count.";
        m_timerCount = m_setting->value(RECORDER_TIME_STARTCOUNTCONFIG).toInt();
    }
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timer->start(400);
    qCDebug(dsrApp) << "Timer started with interval 400ms";
}

void TimeWidget::stop()
{
    qCInfo(dsrApp) << "Stopping time widget";
    disconnect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    qCDebug(dsrApp) << "stop method finished.";
}

//判断是否是wayland协议
bool TimeWidget::isWaylandProtocol()
{
    qCDebug(dsrApp) << "isWaylandProtocol method called.";
    QProcessEnvironment e = QProcessEnvironment::systemEnvironment();

    // check is treeland environment.
    if (e.value(QStringLiteral("DDE_CURRENT_COMPOSITOR")) == QStringLiteral("TreeLand")) {
        qCDebug(dsrApp) << "TreeLand environment detected";
        return false;
    }

    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    bool isWayland = XDG_SESSION_TYPE == QLatin1String("wayland") ||  WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive);
    qCDebug(dsrApp) << "XDG_SESSION_TYPE:" << XDG_SESSION_TYPE << ", WAYLAND_DISPLAY:" << WAYLAND_DISPLAY << ", isWayland:" << isWayland;
    qCDebug(dsrApp) << "isWaylandProtocol method finished.";
    return isWayland;
}

void TimeWidget::showEvent(QShowEvent *e)
{
    qCDebug(dsrApp) << "showEvent method called.";
    // 强制重新刷新 sizePolicy 和 size
    onPositionChanged(m_position);
    DWidget::showEvent(e);
    qCDebug(dsrApp) << "showEvent method finished.";
}
