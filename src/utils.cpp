// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include "constant.h"
#include "dbus_name.h"

#ifdef DWAYLAND_SUPPORT
#include "utils/waylandmousesimulator.h"
#endif

#include <mutex>
#include <dlfcn.h>

#include <QString>
#include <QDir>
#include <QApplication>
#include <QDebug>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QDBusInterface>


#include <QStandardPaths>
#include <QProcess>
#include <QKeyEvent>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "../dbus/com_deepin_daemon_audio.h"
#include "../dbus/com_deepin_daemon_audio_sink.h"
#include "../dbus/com_deepin_daemon_audio_source.h"
#endif

#include <DDialog>
#include <DSysInfo>
#include <DWindowManagerHelper>
#include <DForeignWindow>

// error qtextstream.h must be included before any header file that defines Status
#include <X11/extensions/shape.h>
#include <QtGui/private/qtx11extras_p.h>
#include <QtGui/private/qtguiglobal_p.h>
#include "utils_interface.h"
#include "utils/log.h"

// TODO: qtx11extras-opensource-src没进行qt6适配，后续会补充上
#if (QT_VERSION_MAJOR == 5)
#include <QtX11Extras/QX11Info>
#endif

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

bool Utils::is3rdInterfaceStart = false;
bool Utils::isTabletEnvironment = false;
bool Utils::isWaylandMode = false;
bool Utils::isTreelandMode = false;

bool Utils::forceResetScale = false;

bool Utils::isRootUser = false;
qreal Utils::pixelRatio = 0.0;
bool Utils::isFFmpegEnv = true;
int Utils::themeType = 0;
int Utils::specialRecordingScreenMode = -1;
QString Utils::appName = "";

Utils *Utils::m_utils = nullptr;

Utils::Utils(QObject *parent)
    : utils_interface(AUDIO_NAME,
                     AUDIO_PATH,
                     QDBusConnection::sessionBus(),
                     parent)
{
    qCDebug(dsrApp) << "Utils constructor called.";
    // 初始化代码
}

Utils::~Utils()
{
    qCDebug(dsrApp) << "Utils destructor called.";
}

Utils *Utils::instance()
{
    qCDebug(dsrApp) << "Utils::instance() called.";
    if (m_utils == nullptr) {
        m_utils = new Utils();
        qCDebug(dsrApp) << "New Utils instance created.";
    }
    qCDebug(dsrApp) << "Returning Utils instance.";
    return m_utils;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QString Utils::getCurrentAudioChannel()
{
    const QString serviceName{AUDIO_NAME};

    QScopedPointer<com::deepin::daemon::Audio> audioInterface;
    QScopedPointer<com::deepin::daemon::audio::Sink> defaultSink;

    audioInterface.reset(
        new com::deepin::daemon::Audio(serviceName, AUDIO_PATH, QDBusConnection::sessionBus(), nullptr));

    defaultSink.reset(new com::deepin::daemon::audio::Sink(
        serviceName, audioInterface->defaultSink().path(), QDBusConnection::sessionBus(), nullptr));
    qCDebug(dsrApp) << "Default sink reset.";
    QString str_output = "";
    if (defaultSink->isValid()) {
        QString sinkName = defaultSink->name();
        qCDebug(dsrApp) << "System sound card name:" << sinkName << ".";
        QStringList options;
        options << "-c";
        options << QString("pacmd list-sources | grep -PB 1 %1 | head -n 1 | perl -pe 's/.* //g'").arg(sinkName);

        QProcess process;
        process.start("bash", options);
        process.waitForFinished();
        process.waitForReadyRead();
        str_output = process.readAllStandardOutput();
        qCDebug(dsrApp) << "pacmd command:" << options << ".";
        qCDebug(dsrApp) << "System audio channel number obtained by pacmd command:" << str_output << ".";
        if (str_output.isEmpty()) {
            str_output = audioInterface->defaultSink().path().right(1);
            qCDebug(dsrApp) << "Failed to get system audio channel number via pacmd command! Automatically assigned channel number:" << str_output << ".";
        }
        qCDebug(dsrApp) << "Returning audio channel:" << str_output << ".";
        return str_output;
    } else {
        str_output = "1";
        qCDebug(dsrApp) << "Failed to get system audio channel number! Automatically assigned channel number:" << str_output << ".";
    }
    qCDebug(dsrApp) << "Returning audio channel:" << str_output << ".";
    return str_output;
}
#endif

QString Utils::getQrcPath(QString imageName)
{
    qCDebug(dsrApp) << "getQrcPath() called for image:" << imageName << ".";
    return QString(":/%1").arg(imageName);
}

QSize Utils::getRenderSize(int fontSize, QString string)
{
    qCDebug(dsrApp) << "getRenderSize() called with font size:" << fontSize << ", string:" << string << ".";
    QFont font;
    font.setPointSize(fontSize);
    QFontMetrics fm(font);

    int width = 0;
    int height = 0;
    foreach (auto line, string.split("\n")) {
        int lineWidth = fm.horizontalAdvance(line);
        int lineHeight = fm.height();

        if (lineWidth > width) {
            width = lineWidth;
            qCDebug(dsrApp) << "Max line width updated to:" << width << ".";
        }
        height += lineHeight;
        qCDebug(dsrApp) << "Line height added. Current total height:" << height << ".";
    }

    qCDebug(dsrApp) << "Calculated render size:" << width << "x" << height << ".";
    return QSize(width, height);
}

void Utils::setFontSize(QPainter &painter, int textSize)
{
    qCDebug(dsrApp) << "setFontSize() called with text size:" << textSize << ".";
    QFont font = painter.font();
    font.setPointSize(textSize);
    painter.setFont(font);
    qCDebug(dsrApp) << "Painter font size set.";
}

void Utils::drawTooltipBackground(QPainter &painter, QRect rect, QString textColor, qreal opacity)
{
    qCDebug(dsrApp) << "drawTooltipBackground() called with rect:" << rect << ", text color:" << textColor << ", opacity:" << opacity << ".";
    painter.setOpacity(opacity);
    QPainterPath path;
    path.addRoundedRect(QRectF(rect), Constant::RECTANGLE_RADIUS, Constant::RECTANGLE_RADIUS);
    painter.fillPath(path, QColor(textColor));
    qCDebug(dsrApp) << "Tooltip background path filled.";

    QPen pen(QColor("#000000"));
    painter.setOpacity(0.04);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawPath(path);
    qCDebug(dsrApp) << "Tooltip background path outlined.";
}

void Utils::drawTooltipText(QPainter &painter, QString text, QString textColor, int textSize, QRectF rect)
{
    Q_UNUSED(textColor);
    qCDebug(dsrApp) << "drawTooltipText() called with text:" << text << ", text size:" << textSize << ", rect:" << rect << ".";

    Utils::setFontSize(painter, textSize);
    painter.setOpacity(1);
    painter.setPen(QPen(QGuiApplication::palette().color(QPalette::BrightText)));
    painter.drawText(rect, Qt::AlignCenter, text);
    qCDebug(dsrApp) << "Tooltip text drawn.";
}

void Utils::passInputEvent(int wid)
{
    qCDebug(dsrApp) << "passInputEvent() called for window ID:" << wid << ".";
    // Wayland 事件穿透
    if (Utils::isWaylandMode) {
        qCDebug(dsrApp) << "Wayland mode detected. Setting window handle mask.";
        QWidget *widget = QWidget::find(static_cast<WId>(wid));
        if (widget && widget->windowHandle()) {
            widget->windowHandle()->setMask(QRegion(0, 0, 1, 1));
            qCDebug(dsrApp) << "Window handle mask set to pass input events.";
        }
        return;
    }

    XRectangle *reponseArea = new XRectangle;
    reponseArea->x = 0;
    reponseArea->y = 0;
    reponseArea->width = 0;
    reponseArea->height = 0;
    qCDebug(dsrApp) << "XRectangle response area initialized.";

    XShapeCombineRectangles(
        QX11Info::display(), static_cast<unsigned long>(wid), ShapeInput, 0, 0, reponseArea, 1, ShapeSet, YXBanded);
    qCDebug(dsrApp) << "XShapeCombineRectangles called.";
    delete reponseArea;
    reponseArea = nullptr;
    qCDebug(dsrApp) << "XRectangle response area deleted.";
}

void Utils::setAccessibility(DPushButton *button, const QString name)
{
    qCDebug(dsrApp) << "setAccessibility(DPushButton*) called for button:" << name << ".";
    button->setObjectName(name);
    button->setAccessibleName(name);
}

#if (QT_VERSION_MAJOR == 5)
void Utils::setAccessibility(DImageButton *button, const QString name)
{
    qCDebug(dsrApp) << "setAccessibility(DImageButton*) called for button:" << name << ".";
    button->setObjectName(name);
    button->setAccessibleName(name);
}
#elif (QT_VERSION_MAJOR == 6)
void Utils::setAccessibility(DIconButton *button, const QString name)
{
    qCDebug(dsrApp) << "setAccessibility(DIconButton*) called for button:" << name << ".";
    button->setObjectName(name);
    button->setAccessibleName(name);
}
#endif

void Utils::setAccessibility(QAction *action, const QString name)
{
    qCDebug(dsrApp) << "setAccessibility(QAction*) called for action:" << name << ".";
    action->setObjectName(name);
    // action->setAccessibleName(name);
}

void Utils::getInputEvent(const int wid, const short x, const short y, const unsigned short width, const unsigned short height)
{
    qCDebug(dsrApp) << "getInputEvent() called for window ID:" << wid << ", rect:(" << x << "," << y << "," << width << "," << height << ").";
    if (Utils::isWaylandMode == true) {
        qCDebug(dsrApp) << "Wayland mode detected, skipping getInputEvent().";
        return;
    }
    XRectangle *reponseArea = new XRectangle;
    reponseArea->x = x;
    reponseArea->y = y;
    reponseArea->width = width;
    reponseArea->height = height;
    qCDebug(dsrApp) << "XRectangle response area initialized with input event region.";

    /*
     *XShapeCombineRectangles:
     * param1:
     * param2:
     * param3:
     * param4:
     * param5:
     * param6:为NULL（参数为XRectangle*类型），那整个窗口都将被穿透
     * param7:控制设置穿透的，为0时表示设置鼠标穿透，为1时表示取消鼠标穿透，当取消设置鼠标穿透的时候，必须设置区域，
     * param8:
     * param9:
     */
    XShapeCombineRectangles(
        QX11Info::display(), static_cast<unsigned long>(wid), ShapeInput, 0, 0, reponseArea, 1, ShapeSubtract, YXBanded);
    delete reponseArea;
}

void Utils::cancelInputEvent(const int wid, const short x, const short y, const unsigned short width, const unsigned short height)
{
    qCDebug(dsrApp) << "cancelInputEvent() called for window ID:" << wid << ", rect:(" << x << "," << y << "," << width << "," << height << ").";
    if (Utils::isWaylandMode == true) {
        qCDebug(dsrApp) << "Wayland mode detected, skipping cancelInputEvent().";
        return;
    }
    XRectangle *reponseArea = new XRectangle;
    reponseArea->x = x;
    reponseArea->y = y;
    reponseArea->width = width;
    reponseArea->height = height;
    qCDebug(dsrApp) << "XRectangle response area initialized for cancellation.";

    XShapeCombineRectangles(
        QX11Info::display(), static_cast<unsigned long>(wid), ShapeInput, 0, 0, reponseArea, 1, ShapeSet, YXBanded);
    qCDebug(dsrApp) << "XShapeCombineRectangles called to cancel input event.";
    delete reponseArea;
    reponseArea = nullptr;
    qCDebug(dsrApp) << "XRectangle response area deleted.";
}

void Utils::cancelInputEvent1(
    const int wid, const short x, const short y, const unsigned short width, const unsigned short height)
{
    qCDebug(dsrApp) << "cancelInputEvent1() called for window ID:" << wid << ", rect:(" << x << "," << y << "," << width << "," << height << ").";
    XRectangle *reponseArea = new XRectangle;
    reponseArea->x = x;
    reponseArea->y = y;
    reponseArea->width = width;
    reponseArea->height = height;
    qCDebug(dsrApp) << "XRectangle response area initialized for cancellation (version 1).";

    XShapeCombineRectangles(
        QX11Info::display(), static_cast<unsigned long>(wid), ShapeInput, 0, 0, reponseArea, 0, ShapeSubtract, YXBanded);
    qCDebug(dsrApp) << "XShapeCombineRectangles called to cancel input event (version 1).";
    delete reponseArea;
    reponseArea = nullptr;
    qCDebug(dsrApp) << "XRectangle response area deleted.";
}

// 录屏显示录制时长， 依赖dde-dock。
// 判断系统是否是大于1040, 大于1040才显示录制时长功能。
bool Utils::isSysHighVersion1040()
{
    qCDebug(dsrApp) << "isSysHighVersion1040() called.";
    if (DSysInfo::isDeepin()) {
        bool correct = false;
        QString sysVersion = DSysInfo::minorVersion();
        float version = sysVersion.toFloat(&correct);
        qCDebug(dsrApp) << DSysInfo::uosEditionName() << DSysInfo::uosEditionType() << " System Version:" << sysVersion << ", correct:" << correct << ", split version:" << sysVersion.split('.') << ".";
        if (DSysInfo::UosProfessional == DSysInfo::uosEditionType() || DSysInfo::UosEducation == DSysInfo::uosEditionType() ||
            DSysInfo::UosMilitary == DSysInfo::uosEditionType()) {
            const float versionProfessional = 1040;
            if (correct && (version >= versionProfessional)) {
                qCDebug(dsrApp) << "System version (Professional/Education/Military) is >= 1040, returning true.";
                return true;
            }
            qCDebug(dsrApp) << "System version (Professional/Education/Military) is < 1040 or incorrect, returning false.";
        } else if (DSysInfo::UosHome == DSysInfo::uosEditionType()) {
            const float versionHome = 21.3f;
            if (correct && (version >= versionHome)) {
                qCDebug(dsrApp) << "System version (Home) is >= 21.3, returning true.";
                return true;
            }
            qCDebug(dsrApp) << "System version (Home) is < 21.3 or incorrect, returning false.";
        } else if (DSysInfo::UosCommunity == DSysInfo::uosEditionType()) {
            const float versionCommunity = 20.6f;
            QStringList v = sysVersion.split('.');
            if (!correct && v.size() > 2) {
                // 社区版，版本号可能有20.6.1，只取前面的两位大版本号
                version = (QString("%1.%2").arg(v[0]).arg(v[1])).toFloat(&correct);
                qCDebug(dsrApp) << "Fix System Version (Community):" << version << ", correct:" << correct << ".";
            }
            if (correct && (version >= versionCommunity)) {
                qCDebug(dsrApp) << "System version (Community) is >= 20.6, returning true.";
                return true;
            }
            qCDebug(dsrApp) << "System version (Community) is < 20.6 or incorrect, returning false.";
        }
    }
    qCDebug(dsrApp) << "System is not Deepin or does not meet version requirements, returning false.";
    return false;
}

/**
   @brief Use different interfaces depending on the system version，
          V23 and later versions use version 2.0, otherwise version 1.0.
          Most DBus interfaces are affected.
   @return Current system edition greater or equal V23
 */
bool Utils::isSysGreatEqualV23()
{
    qCDebug(dsrApp) << "isSysGreatEqualV23() called.";
    static bool kGreaterEqualV23 = false;
    static std::once_flag kGraterFlag;
    std::call_once(kGraterFlag, [&]() {
        qCDebug(dsrApp) << "Calling once for kGraterFlag.";
        if (DSysInfo::isDeepin()) {
            static const int kV23Edition = 23;
            kGreaterEqualV23 = bool(DSysInfo::majorVersion().toInt() >= kV23Edition);
            qCDebug(dsrApp) << "System is Deepin, kGreaterEqualV23 set to:" << kGreaterEqualV23 << ".";
        }
    });
    qCDebug(dsrApp) << "Returning kGreaterEqualV23:" << kGreaterEqualV23 << ".";
    return kGreaterEqualV23;
}

void Utils::showCurrentSys()
{
    qCInfo(dsrApp) << "isDeepin: " << DSysInfo::isDeepin();
    qCInfo(dsrApp) << "isDDE: " << DSysInfo::isDDE();
    qCInfo(dsrApp) << "SystemName: " << DSysInfo::uosSystemName();
    qCInfo(dsrApp) << "EditionName: " << DSysInfo::uosEditionName();
    qCInfo(dsrApp) << "ProductTypeName: " << DSysInfo::uosProductTypeName();
    qCInfo(dsrApp) << "SystemVersion: " << DSysInfo::minorVersion();
    qCInfo(dsrApp) << "uosEditionType: " << DSysInfo::uosEditionType();
    // qDebug() << "spVersion: " << DSysInfo::spVersion();
    // qDebug() << "udpateVersion: " << DSysInfo::udpateVersion();
    // qDebug() << "majorVersion: " << DSysInfo::majorVersion();
    // qDebug() << "majorVersion: " << DSysInfo::majorVersion();
}

void Utils::enableXGrabButton()
{
    qCDebug(dsrApp) << "enableXGrabButton() called.";
    if (Utils::isWaylandMode == true) {
        qCDebug(dsrApp) << "Wayland mode detected, skipping enableXGrabButton().";
        return;
    }
    // extern int XGrabButton(
    //     Display *      /* display */,
    //     unsigned int  /* button */,
    //     unsigned int  /* modifiers */,
    //     Window        /* grab_window */,
    //     Bool      /* owner_events */,
    //     unsigned int  /* event_mask */,
    //     int           /* pointer_mode */,
    //     int           /* keyboard_mode */,
    //     Window        /* confine_to */,
    //     Cursor        /* cursor */
    //);
    XGrabButton(QX11Info::display(),
                Button1,
                AnyModifier,
                DefaultRootWindow(QX11Info::display()),
                true,
                ButtonPressMask | ButtonReleaseMask,
                GrabModeAsync,
                GrabModeAsync,
                None,
                None);
    qCDebug(dsrApp) << "XGrabButton called for Button1.";
    XGrabButton(QX11Info::display(),
                Button2,
                AnyModifier,
                DefaultRootWindow(QX11Info::display()),
                true,
                ButtonPressMask | ButtonReleaseMask,
                GrabModeAsync,
                GrabModeAsync,
                None,
                None);
    qCDebug(dsrApp) << "XGrabButton called for Button2.";
    XGrabButton(QX11Info::display(),
                Button3,
                AnyModifier,
                DefaultRootWindow(QX11Info::display()),
                true,
                ButtonPressMask | ButtonReleaseMask,
                GrabModeAsync,
                GrabModeAsync,
                None,
                None);
    qCDebug(dsrApp) << "XGrabButton called for Button3.";
}

void Utils::disableXGrabButton()
{
    qCDebug(dsrApp) << "disableXGrabButton() called.";
    if (Utils::isWaylandMode == true) {
        qCDebug(dsrApp) << "Wayland mode detected, skipping disableXGrabButton().";
        return;
    }
    XUngrabButton(QX11Info::display(), true, AnyModifier, DefaultRootWindow(QX11Info::display()));
    qCDebug(dsrApp) << "XUngrabButton called.";
}

void Utils::getAllWindowInfo(
    const quint32 winId, const int width, const int height, QList<QRect> &windowRects, QList<QString> &windowNames)
{
    qCDebug(dsrApp) << "getAllWindowInfo() called for window ID:" << winId << ", width:" << width << ", height:" << height << ".";
    Dtk::Gui::DForeignWindow *prewindow = nullptr;
    for (quint32 wid : Dtk::Gui::DWindowManagerHelper::instance()->currentWorkspaceWindowIdList()) {
        qCDebug(dsrApp) << "Processing window ID:" << wid << ".";
        if (wid == winId) {
            qCDebug(dsrApp) << "Skipping current window ID:" << wid << ".";
            continue;
        }
        if (prewindow && isSysHighVersion1040()) {
            qCDebug(dsrApp) << "Deleting prewindow as system version is high.";
            delete prewindow;
            prewindow = nullptr;
        }

        Dtk::Gui::DForeignWindow *window = Dtk::Gui::DForeignWindow::fromWinId(wid);  // sanitizer dtk
        qCDebug(dsrApp) << "DForeignWindow created for window ID:" << wid << ".";

        prewindow = window;

        // 判断窗口是否被最小化
        if (window->windowState() == Qt::WindowState::WindowMinimized) {
            qCDebug(dsrApp) << "Window ID:" << wid << "is minimized, skipping.";
            continue;
        }

        if (window) {
            int t_tempWidth = 0;
            int t_tempHeight = 0;
            // window:后面代码有使用
            // window->deleteLater();
            // 修改部分窗口显示不全，截图框识别问题
            //            qCDebug(dsrApp)  << "\n窗口名称: >>>> " << window->wmClass()
            //                      << "\nx: " <<  window->frameGeometry().x()
            //                      << "\ny: " <<  window->frameGeometry().y()
            //                      << "\nwidth: " <<  window->frameGeometry().width()
            //                      << "\nheight: " <<  window->frameGeometry().height()   ;
            // x坐标小于0时
            if (window->frameGeometry().x() < 0) {
                qCDebug(dsrApp) << "Window x-coordinate is less than 0.";
                if (window->frameGeometry().y() < 0) {
                    // x,y为负坐标情况
                    qCDebug(dsrApp) << "Window x and y coordinates are negative.";
                    t_tempWidth = window->frameGeometry().width() + window->frameGeometry().x();
                    t_tempHeight = window->frameGeometry().height() + window->frameGeometry().y();

                    // windowRects << Dtk::Wm::WindowRect {0, 0, t_tempWidth, t_tempHeight};
                    windowRects << QRect(0, 0, t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    qCDebug(dsrApp) << "Added window:" << window->wmClass() << ", rect:(" << 0 << "," << 0 << "," << t_tempWidth << "," << t_tempHeight << ").";
                    continue;
                } else if (window->frameGeometry().y() >= 0 &&
                           window->frameGeometry().y() <= height - window->frameGeometry().height()) {
                    // x为负坐标，y在正常屏幕区间内
                    t_tempWidth = window->frameGeometry().width() + window->frameGeometry().x();
                    t_tempHeight = window->frameGeometry().height();

                    // windowRects << Dtk::Wm::WindowRect {0, window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(0, window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() > height - window->frameGeometry().height()) {
                    // x为负坐标，y方向窗口超出屏幕底部
                    t_tempWidth = window->frameGeometry().width() + window->frameGeometry().x();
                    t_tempHeight = height - window->frameGeometry().y();

                    //                        windowRects << Dtk::Wm::WindowRect {0, window->frameGeometry().y(), t_tempWidth,
                    //                        t_tempHeight};
                    windowRects << QRect(0, window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                }
            }

            // x坐标位于正常屏幕区间时
            else if (window->frameGeometry().x() >= 0 && window->frameGeometry().x() <= width - window->frameGeometry().width()) {
                qCDebug(dsrApp) << "Window x-coordinate is within the normal screen range.";
                if (window->frameGeometry().y() < 0) {
                    // y为负坐标情况
                    t_tempWidth = window->frameGeometry().width();
                    t_tempHeight = window->frameGeometry().height() + window->frameGeometry().y();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), 0, t_tempWidth,
                    //                        t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), 0, t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() >= 0 &&
                           window->frameGeometry().y() <= height - window->frameGeometry().height()) {
                    // y在正常屏幕区间内
                    t_tempWidth = window->frameGeometry().width();
                    t_tempHeight = window->frameGeometry().height();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(),
                    //                        window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() > height - window->frameGeometry().height()) {
                    // y方向窗口超出屏幕底部
                    t_tempWidth = window->frameGeometry().width();
                    t_tempHeight = height - window->frameGeometry().y();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(),
                    //                        window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                }
            }

            // x方向窗口超出屏幕右侧区域
            else if (window->frameGeometry().x() > width - window->frameGeometry().width()) {
                qCDebug(dsrApp) << "Window x-coordinate is greater than the normal screen range.";
                if (window->frameGeometry().y() < 0) {
                    // y为负坐标情况
                    t_tempWidth = width - window->frameGeometry().x();
                    t_tempHeight = window->frameGeometry().height() + window->frameGeometry().y();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), 0, t_tempWidth,
                    //                        t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), 0, t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() >= 0 &&
                           window->frameGeometry().y() <= height - window->frameGeometry().height()) {
                    // y在正常屏幕区间内
                    t_tempWidth = width - window->frameGeometry().x();
                    t_tempHeight = window->frameGeometry().height();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(),
                    //                        window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() > height - window->frameGeometry().height()) {
                    // y方向窗口超出屏幕底部
                    t_tempWidth = width - window->frameGeometry().x();
                    t_tempHeight = height - window->frameGeometry().y();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(),
                    //                        window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                }
            }
        }
    }
    if (prewindow && isSysHighVersion1040()) {
        qCDebug(dsrApp) << "prewindow is not nullptr. and 1040 system version. deleting prewindow.";
        delete prewindow;
        prewindow = nullptr;
    }
}

bool Utils::checkCpuIsZhaoxin()
{
    qCInfo(dsrApp) << "checkCpuIsZhaoxin() called.";
    QProcess process;
    process.start("lscpu");
    process.waitForFinished();
    process.waitForReadyRead();
    QString comStr = process.readAllStandardOutput();
    process.close();
    QStringList lines = comStr.split('\n');
    QStringList filteredLines;

    // 过滤包含指定关键字的行
    for (const QString &line : lines) {
        if (line.contains("CentaurHauls", Qt::CaseInsensitive)) {
            filteredLines.append(line);
        }
    }
    if (filteredLines.isEmpty()) {
        qCInfo(dsrApp) << "CPU is not zhaoxin.";
        return false;
    }
    qCInfo(dsrApp) << "CPU is zhaoxin.";
    return true;
}

// 获取处理器名称
QString Utils::getCpuModelName()
{
    qCInfo(dsrApp) << "getCpuModelName() called.";
    if (DSysInfo::cpuModelName().isNull()) {
        qCInfo(dsrApp) << "CPU model name is null.";
        return "";
    }
    qCInfo(dsrApp) << "CPU model name is:" << DSysInfo::cpuModelName();
    return DSysInfo::cpuModelName();
}

void Utils::notSupportWarn()
{
    qCInfo(dsrApp) << "notSupportWarn() called.";
    DDialog warnDlg;
    warnDlg.setIcon(QIcon::fromTheme("deepin-screen-recorder"));
    warnDlg.setMessage(tr("Screen recording is not supported at present"));
    warnDlg.addSpacing(20);
    warnDlg.addButton(tr("Exit"));
    warnDlg.exec();
}

// 传入屏幕上理论未经缩放的点，获取缩放后实际的点
QPoint Utils::getPosWithScreen(QPoint pos)
{
    qCInfo(dsrApp) << "getPosWithScreen() called.";
    QPoint dpos;
    QList<ScreenInfo> screensInfo;
    // 获取所有实际屏幕的信息
    screensInfo = getScreensInfo();
    qreal pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    for (int i = 0; i < screensInfo.size(); i++) {
        // 判断当前点在哪块屏幕上
        if (pos.x() > screensInfo[i].x && pos.x() < screensInfo[i].x + screensInfo[i].width && pos.y() > screensInfo[i].y &&
            pos.y() < screensInfo[i].y + screensInfo[i].height) {
            qCInfo(dsrApp) << "screenInfo: " << screensInfo[i].toString();
            dpos.setX(static_cast<int>((pos.x() - screensInfo[i].x) / pixelRatio + screensInfo[i].x));
            dpos.setY(static_cast<int>((pos.y() - screensInfo[i].y) / pixelRatio) + +screensInfo[i].y);
        }
    }
    qCDebug(dsrApp) << "Position" << pos << "not found within any screen. Returning original position:" << dpos;
    return dpos;
}

// 传入屏幕上已经缩放后的点，获取理论上实际的点
QPoint Utils::getPosWithScreenP(QPoint pos)
{
    qCInfo(dsrApp) << "getPosWithScreenP() called.";
    QPoint dpos;
    QList<ScreenInfo> screensInfo;
    // 获取所有实际屏幕的信息
    screensInfo = getScreensInfo();
    qreal pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    for (int i = 0; i < screensInfo.size(); i++) {
        // 判断当前点在哪块屏幕上
        if (pos.x() > screensInfo[i].x && pos.x() < screensInfo[i].x + screensInfo[i].width && pos.y() > screensInfo[i].y &&
            pos.y() < screensInfo[i].y + screensInfo[i].height) {
            dpos.setX(static_cast<int>((pos.x() - screensInfo[i].x) * pixelRatio + screensInfo[i].x));
            dpos.setY(static_cast<int>((pos.y() - screensInfo[i].y) * pixelRatio + screensInfo[i].y));
        }
    }
    qCDebug(dsrApp) << "Position" << pos << "not found within any screen. Returning original position:" << dpos;
    return dpos;
}

QList<Utils::ScreenInfo> Utils::getScreensInfo()
{
    qCInfo(dsrApp) << "getScreensInfo() called.";
    // 获取所有实际屏幕的信息
    QList<QScreen *> screenList = qApp->screens();
    qreal pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    QList<ScreenInfo> screensInfo;
    /**
     * 例如：显示器实际屏幕参数为1920*1080记为 t1，调整缩放比例为1.25记为 p,那么此时显示器显示屏幕大小将变为1536*864记为 t2，
     * 那么此时在 t2的点要变成t1上的点，需要 t2*p = t1,在多屏的情况且需要考虑究竟在那个屏幕上
     */
    // 将实际屏幕信息根据缩放比列转化为理论上屏幕信息
    int hTotal = 0;
    for (auto it = screenList.constBegin(); it != screenList.constEnd(); ++it) {
        QRect rect = (*it)->geometry();
        // qDebug() << (*it)->name() << rect;
        ScreenInfo screenInfo;
        screenInfo.x = rect.x();  // 屏幕的起始x坐标始终是正确的，不管是否经过缩放
        screenInfo.y = rect.y();  // 屏幕的起始y坐标始终是正确的，不管是否经过缩放
        screenInfo.height = static_cast<int>(rect.height() * pixelRatio);
        screenInfo.width = static_cast<int>(rect.width() * pixelRatio);
        screenInfo.name = (*it)->name();
        hTotal += screenInfo.height;
        screensInfo.append(screenInfo);
    }
    qCInfo(dsrApp) << "screensInfo size: " << screensInfo.size();
    return screensInfo;
}

// 通过键盘控光标移动
void Utils::cursorMove(QPoint currentCursor, QKeyEvent *keyEvent)
{
    qCInfo(dsrApp) << "cursorMove() called with current cursor";
    QPoint pos(currentCursor);
    // qDebug() << "pos() >>>> 1 " << pos;
    if (keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_A) {
        pos.setX(std::max(pos.x() - 1, 0));
    } else if (keyEvent->key() == Qt::Key_Right || keyEvent->key() == Qt::Key_D) {
        pos.setX(pos.x() + 1);
    } else if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_W) {
        pos.setY(std::max(pos.y() - 1, 0));
    } else if (keyEvent->key() == Qt::Key_Down || keyEvent->key() == Qt::Key_S) {
        pos.setY(pos.y() + 1);
    } else {
        qCInfo(dsrApp) << "the key is " << keyEvent->key() << " (" << keyEvent->text() << ")";
    }

    if (Utils::isWaylandMode) {
        qCInfo(dsrApp) << "isWaylandMode is true, use WaylandMouseSimulator to move cursor.";
#ifdef KF5_WAYLAND_FLAGE_ON
#ifdef DWAYLAND_SUPPORT
        WaylandMouseSimulator::instance()->setCursorPoint(QPointF(pos.x() * Utils::pixelRatio, pos.y() * Utils::pixelRatio));
#else
        // TODO: implement in kf5wayland environment
#endif  // DWAYLAND_SUPPORT
#endif  // KF5_WAYLAND_FLAGE_ON
    } else {
        qCInfo(dsrApp) << "isWaylandMode is false, use QCursor to move cursor.";
        QCursor::setPos(pos);
    }
}
