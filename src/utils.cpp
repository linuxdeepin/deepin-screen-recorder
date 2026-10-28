// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include "constant.h"
#include <DDialog>
#include <DSysInfo>
#include <DWindowManagerHelper>
#include <DForeignWindow>


#include <QString>
#include <QDir>
#include <QApplication>
#include <QDebug>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QtDBus>
#include <QElapsedTimer>
#include <QMutex>
#include <QtX11Extras/QX11Info>
#include <QStandardPaths>
#include <QProcess>

#include <X11/extensions/shape.h>
#include <dlfcn.h>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

bool Utils::is3rdInterfaceStart = false;
bool Utils::isTabletEnvironment = false;
bool Utils::isWaylandMode = false;
bool Utils::isRootUser = false;
bool Utils::isFFmpegEnv = true;
int Utils::themeType = 0;
int Utils::specialRecordingScreenMode = -1;
int Utils::waylandFrameInterpolation = 0;
QString Utils::ffmpegFilterString = "fps=%1";

QString Utils::getQrcPath(QString imageName)
{
    return QString(":/%1").arg(imageName);
}

QSize Utils::getRenderSize(int fontSize, QString string)
{
    QFont font;
    font.setPointSize(fontSize);
    QFontMetrics fm(font);

    int width = 0;
    int height = 0;
    foreach (auto line, string.split("\n")) {
        int lineWidth = fm.width(line);
        int lineHeight = fm.height();

        if (lineWidth > width) {
            width = lineWidth;
        }
        height += lineHeight;
    }

    return QSize(width, height);
}

void Utils::setFontSize(QPainter &painter, int textSize)
{
    QFont font = painter.font() ;
    font.setPointSize(textSize);
    painter.setFont(font);
}


void Utils::drawTooltipBackground(QPainter &painter, QRect rect, QString textColor, qreal opacity)
{
    painter.setOpacity(opacity);
    QPainterPath path;
    path.addRoundedRect(QRectF(rect), Constant::RECTANGLE_RADIUS, Constant::RECTANGLE_RADIUS);
    painter.fillPath(path, QColor(textColor));

    QPen pen(QColor("#000000"));
    painter.setOpacity(0.04);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawPath(path);
}

void Utils::drawTooltipText(QPainter &painter, QString text, QString textColor, int textSize, QRectF rect)
{
    Q_UNUSED(textColor);

    Utils::setFontSize(painter, textSize);
    painter.setOpacity(1);
    painter.setPen(QPen(QGuiApplication::palette().color(QPalette::BrightText)));
    painter.drawText(rect, Qt::AlignCenter, text);
}

void Utils::passInputEvent(int wid)
{
    // Wayland 事件穿透
    if (Utils::isWaylandMode) {
        QWidget *widget = QWidget::find(static_cast<WId>(wid));
        if (widget && widget->windowHandle()) {
            widget->windowHandle()->setMask(QRegion(0, 0, 1, 1));
        }
        return;
    }


    XRectangle *reponseArea = new XRectangle;
    reponseArea->x = 0;
    reponseArea->y = 0;
    reponseArea->width = 0;
    reponseArea->height = 0;

    XShapeCombineRectangles(QX11Info::display(), static_cast<unsigned long>(wid), ShapeInput, 0, 0, reponseArea, 1, ShapeSet, YXBanded);
    delete reponseArea;

}

void Utils::setAccessibility(DPushButton *button, const QString name)
{
    button->setObjectName(name);
    button->setAccessibleName(name);
}
void Utils::setAccessibility(DImageButton *button, const QString name)
{
    button->setObjectName(name);
    button->setAccessibleName(name);
}
void Utils::setAccessibility(QAction *action, const QString name)
{
    action->setObjectName(name);
    //action->setAccessibleName(name);
}

void Utils::getInputEvent(const int wid, const short x, const short y, const unsigned short width, const unsigned short height)
{
    if (Utils::isWaylandMode == true)
        return;
    XRectangle *reponseArea = new XRectangle;
    reponseArea->x = x;
    reponseArea->y = y;
    reponseArea->width = width;
    reponseArea->height = height;

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
    XShapeCombineRectangles(QX11Info::display(), static_cast<unsigned long>(wid), ShapeInput, 0, 0, reponseArea, 1, ShapeSubtract, YXBanded);
    delete reponseArea;

}

void Utils::cancelInputEvent(const int wid, const short x, const short y, const unsigned short width, const unsigned short height)
{
    if (Utils::isWaylandMode == true)
        return;
    XRectangle *reponseArea = new XRectangle;
    reponseArea->x = x;
    reponseArea->y = y;
    reponseArea->width = width;
    reponseArea->height = height;

    XShapeCombineRectangles(QX11Info::display(), static_cast<unsigned long>(wid), ShapeInput, 0, 0, reponseArea, 1, ShapeSet, YXBanded);
    delete reponseArea;

}

void Utils::cancelInputEvent1(const int wid, const short x, const short y, const unsigned short width, const unsigned short height)
{
    XRectangle *reponseArea = new XRectangle;
    reponseArea->x = x;
    reponseArea->y = y;
    reponseArea->width = width;
    reponseArea->height = height;

    XShapeCombineRectangles(QX11Info::display(), static_cast<unsigned long>(wid), ShapeInput, 0, 0, reponseArea, 0, ShapeSubtract, YXBanded);
    delete reponseArea;

}


// 录屏显示录制时长， 依赖dde-dock。
// 判断系统是否是大于1040, 大于1040才显示录制时长功能。
bool Utils::isSysHighVersion1040()
{
    if (DSysInfo::isDeepin()) {
        bool correct = false;
        QString sysVersion = DSysInfo::minorVersion();
        float version = sysVersion.toFloat(&correct);
        qDebug() << "System Version:" << sysVersion << correct << sysVersion.split('.');
        if (DSysInfo::UosProfessional == DSysInfo::uosEditionType() || DSysInfo::UosEducation == DSysInfo::uosEditionType() || DSysInfo::UosMilitary == DSysInfo::uosEditionType()) {
            const float versionProfessional = 1040;
            if (correct && (version >= versionProfessional)) {
                return true;
            }
        } else if (DSysInfo::UosHome == DSysInfo::uosEditionType()) {
            const float versionHome = 21.3f;
            if (correct && (version >= versionHome)) {
                return true;
            }
        } else if (DSysInfo::UosCommunity == DSysInfo::uosEditionType()) {
            const float versionCommunity = 20.6f;
            QStringList v = sysVersion.split('.');
            if (!correct && v.size() > 2) {
                // 社区版，版本号可能有20.6.1，只取前面的两位大版本号
                version = (QString("%1.%2").arg(v[0]).arg(v[1])).toFloat(&correct);
                qDebug() << "Fix System Version:" << version << correct;
            }
            if (correct && (version >= versionCommunity)) {
                return true;
            }
        }
    }
    return false;
}

void Utils::showCurrentSys()
{
    qInfo() << "isDeepin: " << DSysInfo::isDeepin();
    qInfo() << "isDDE: " << DSysInfo::isDDE();
    qInfo() << "SystemName: " << DSysInfo::uosSystemName();
    qInfo() << "EditionName: " << DSysInfo::uosEditionName();
    qInfo() << "ProductTypeName: " << DSysInfo::uosProductTypeName();
    qInfo() << "SystemVersion: " << DSysInfo::minorVersion();
    qInfo() << "uosEditionType: " << DSysInfo::uosEditionType();
    qInfo() << "cpuArchitecture: " << QSysInfo::currentCpuArchitecture();
    //qDebug() << "spVersion: " << DSysInfo::spVersion();
    //qDebug() << "udpateVersion: " << DSysInfo::udpateVersion();
    //qDebug() << "majorVersion: " << DSysInfo::majorVersion();
    //qDebug() << "majorVersion: " << DSysInfo::majorVersion();
    qInfo() << "isVirtualMachine: " << isRunningInVirtualMachine();
}

//Detect whether running in a virtual machine
bool Utils::isRunningInVirtualMachine(bool forceVirtualMachine)
{
    QProcess process;
    process.start("/usr/bin/systemd-detect-virt");
    process.waitForFinished();
    QString output = process.readAllStandardOutput().trimmed();

    // Print the output of systemd-detect-virt command
    qInfo() << "systemd-detect-virt output: " << output;

    // none: not a virtual machine
    // kvm, vmware, virtualbox, xen, qemu, etc.: virtual machine
    // empty output: also considered as not a virtual machine
    if (forceVirtualMachine) {
        qInfo() << "Force treat as virtual machine";
        return true;
    }

    if (output.isEmpty() || output.toLower() == "none") {
        qInfo() << "Running on physical machine, not a virtual machine";
        return false;
    } else {
        qInfo() << "Running in a virtual machine, type:" << output;
        return true;
    }
}
void Utils::enableXGrabButton()
{
    if (Utils::isWaylandMode == true)
        return;
    //extern int XGrabButton(
    //    Display *      /* display */,
    //    unsigned int  /* button */,
    //    unsigned int  /* modifiers */,
    //    Window        /* grab_window */,
    //    Bool      /* owner_events */,
    //    unsigned int  /* event_mask */,
    //    int           /* pointer_mode */,
    //    int           /* keyboard_mode */,
    //    Window        /* confine_to */,
    //    Cursor        /* cursor */
    //);
    XGrabButton(QX11Info::display(), Button1, AnyModifier, DefaultRootWindow(QX11Info::display())
                , true, ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(QX11Info::display(), Button2, AnyModifier, DefaultRootWindow(QX11Info::display())
                , true, ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(QX11Info::display(),  Button3, AnyModifier, DefaultRootWindow(QX11Info::display())
                , true, ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
}

void Utils::disableXGrabButton()
{
    if (Utils::isWaylandMode == true)
        return;
    XUngrabButton(QX11Info::display(), AnyButton, AnyModifier, DefaultRootWindow(QX11Info::display()));

}

void Utils::getAllWindowInfo(const quint32 winId, const int width, const int height, QList<QRect> &windowRects, QList<QString> &windowNames)
{
    Dtk::Gui::DForeignWindow *prewindow = nullptr;
    const bool canRelease = isSysHighVersion1040();
    for (quint32 wid : Dtk::Gui::DWindowManagerHelper::instance()->currentWorkspaceWindowIdList()) {
        if (wid == winId) continue;
        if (prewindow && canRelease) {
            delete prewindow;
            prewindow = nullptr;
        }

        Dtk::Gui::DForeignWindow *window = Dtk::Gui::DForeignWindow::fromWinId(wid);//sanitizer dtk

        prewindow = window;

        //判断窗口是否被最小化
        if (window->windowState() == Qt::WindowState::WindowMinimized) {
            continue;
        }

        if (window) {
            int t_tempWidth = 0;
            int t_tempHeight = 0;
            //window:后面代码有使用
            //window->deleteLater();
            //修改部分窗口显示不全，截图框识别问题
            qDebug()  << "\n窗口名称: >>>> " << window->wmClass()
                      << "\nx: " <<  window->frameGeometry().x()
                      << "\ny: " <<  window->frameGeometry().y()
                      << "\nwidth: " <<  window->frameGeometry().width()
                      << "\nheight: " <<  window->frameGeometry().height()   ;
            //x坐标小于0时
            if (window->frameGeometry().x() < 0) {
                if (window->frameGeometry().y() < 0) {

                    //x,y为负坐标情况
                    t_tempWidth = window->frameGeometry().width() + window->frameGeometry().x();
                    t_tempHeight = window->frameGeometry().height() + window->frameGeometry().y();

                    //windowRects << Dtk::Wm::WindowRect {0, 0, t_tempWidth, t_tempHeight};
                    windowRects << QRect(0, 0, t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() >= 0 && window->frameGeometry().y() <= height - window->frameGeometry().height()) {
                    //x为负坐标，y在正常屏幕区间内
                    t_tempWidth = window->frameGeometry().width() + window->frameGeometry().x();
                    t_tempHeight = window->frameGeometry().height();

                    //windowRects << Dtk::Wm::WindowRect {0, window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(0, window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() > height - window->frameGeometry().height()) {
                    //x为负坐标，y方向窗口超出屏幕底部
                    t_tempWidth = window->frameGeometry().width() + window->frameGeometry().x();
                    t_tempHeight = height - window->frameGeometry().y();

                    //                        windowRects << Dtk::Wm::WindowRect {0, window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(0, window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                }
            }

            //x坐标位于正常屏幕区间时
            else if (window->frameGeometry().x() >= 0 && window->frameGeometry().x() <= width - window->frameGeometry().width()) {
                if (window->frameGeometry().y() < 0) {
                    //y为负坐标情况
                    t_tempWidth = window->frameGeometry().width();
                    t_tempHeight = window->frameGeometry().height() + window->frameGeometry().y();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), 0, t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), 0, t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() >= 0 && window->frameGeometry().y() <= height - window->frameGeometry().height()) {
                    //y在正常屏幕区间内
                    t_tempWidth = window->frameGeometry().width();
                    t_tempHeight = window->frameGeometry().height();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() > height - window->frameGeometry().height()) {
                    //y方向窗口超出屏幕底部
                    t_tempWidth = window->frameGeometry().width();
                    t_tempHeight = height - window->frameGeometry().y();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                }
            }

            //x方向窗口超出屏幕右侧区域
            else if (window->frameGeometry().x() > width - window->frameGeometry().width()) {
                if (window->frameGeometry().y() < 0) {
                    //y为负坐标情况
                    t_tempWidth = width - window->frameGeometry().x();
                    t_tempHeight = window->frameGeometry().height() + window->frameGeometry().y();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), 0, t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), 0, t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() >= 0 && window->frameGeometry().y() <= height - window->frameGeometry().height()) {
                    //y在正常屏幕区间内
                    t_tempWidth = width - window->frameGeometry().x();
                    t_tempHeight = window->frameGeometry().height();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                } else if (window->frameGeometry().y() > height - window->frameGeometry().height()) {
                    //y方向窗口超出屏幕底部
                    t_tempWidth = width - window->frameGeometry().x();
                    t_tempHeight = height - window->frameGeometry().y();

                    //                        windowRects << Dtk::Wm::WindowRect {window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight};
                    windowRects << QRect(window->frameGeometry().x(), window->frameGeometry().y(), t_tempWidth, t_tempHeight);
                    windowNames << window->wmClass();
                    continue;
                }
            }
        }
    }
    if (prewindow && isSysHighVersion1040()) {
        delete prewindow;
        prewindow = nullptr;
    }
}


bool Utils::checkCpuIsZhaoxin()
{
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
        return false;
    }
    return true;
}

//获取处理器名称
QString Utils::getCpuModelName()
{
    if (DSysInfo::cpuModelName().isNull()) {
        return "";
    }
    return DSysInfo::cpuModelName();
}

void Utils::notSupportWarn()
{
    DDialog warnDlg;
    warnDlg.setIcon(QIcon::fromTheme("deepin-screen-recorder"));
    warnDlg.setMessage(tr("Screen recording is not supported at present"));
    warnDlg.addSpacing(20);
    warnDlg.addButton(tr("Exit"));
    warnDlg.exec();
}

//传入屏幕上理论未经缩放的点，获取缩放后实际的点
// 从指定 DBus 会话服务读取各屏幕实际缩放系数表（按屏名索引）；查询失败返回空表
static QMap<QString, double> queryScreenScaleFactorsFromDBus(const QString &service, const QString &path,
                                                             const QString &interface)
{
    // 必须：QDBusReply<QMap<QString,double>> 反序列化 a{sd} 需要注册 metatype，
    // 否则恒无效、静默回落 QScreen（返回整数 dpr=2，丢失小数）
    qDBusRegisterMetaType<QMap<QString, double>>();

    QMap<QString, double> factors;
    QDBusInterface iface(service, path, interface, QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        qDebug() << "invalid dbus interface:" << service << path << interface;
        return factors;
    }

    QDBusReply<QMap<QString, double>> reply = iface.call(QStringLiteral("GetScreenScaleFactors"));
    if (!reply.isValid() || reply.value().isEmpty()) {
        qDebug() << "get screen scale factors failed from" << service << path << reply.error();
        return factors;
    }
    return reply.value();
}

// 各屏缩放系数表（按屏名索引），带 500ms 缓存；该函数会被鼠标事件等高频路径调用
static QMap<QString, double> allScreenScaleFactors()
{
    static QMap<QString, double> cachedFactors;
    static QElapsedTimer cacheTimer;
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    const qint64 kCacheTimeoutMs = 500;
    if (!cachedFactors.isEmpty() && cacheTimer.isValid() && !cacheTimer.hasExpired(kCacheTimeoutMs)) {
        return cachedFactors;
    }

    QMap<QString, double> factors = queryScreenScaleFactorsFromDBus(QStringLiteral("com.deepin.daemon.Display"),
                                                                    QStringLiteral("/com/deepin/XSettings"),
                                                                    QStringLiteral("com.deepin.XSettings"));
    if (factors.isEmpty()) {
        factors = queryScreenScaleFactorsFromDBus(QStringLiteral("com.deepin.daemon.Appearance"),
                                                  QStringLiteral("/com/deepin/daemon/Appearance"),
                                                  QStringLiteral("com.deepin.daemon.Appearance"));
    }

    if (!factors.isEmpty()) {
        cachedFactors = factors;
        cacheTimer.start();
    }
    return factors;
}

// 按屏名（或单屏兜底）从系数表取该屏缩放比；表不可用时回落 QScreen::devicePixelRatio()
static qreal scaleFactorForScreen(const QScreen *screen)
{
    if (!screen) {
        return 1.0;
    }

    const QMap<QString, double> factors = allScreenScaleFactors();
    if (factors.contains(screen->name())) {
        return factors.value(screen->name());
    }
    // 未找到同名屏幕，但系数表只有一项时直接采用（单屏时屏名对不上也不影响）
    if (factors.size() == 1) {
        return factors.constBegin().value();
    }

    const qreal fallback = screen->devicePixelRatio();
    return (fallback > 0.0 && fallback <= 10.0) ? fallback : 1.0;
}

/**
 * @brief 获取主屏缩放比例，优先级：
 *        1) com.deepin.daemon.Display 服务 /com/deepin/XSettings 对象（接口 com.deepin.XSettings）
 *        2) com.deepin.daemon.Appearance 服务（接口 com.deepin.daemon.Appearance）
 *        均不可用时回落到 QScreen::devicePixelRatio()。
 */
qreal Utils::devicePixelRatioF()
{
    QScreen *screen = qApp->primaryScreen();
    if (!screen) {
        return 1.0;
    }
    return scaleFactorForScreen(screen);
}

qreal Utils::screenFactor(const QScreen *screen)
{
    return scaleFactorForScreen(screen);
}

QPoint Utils::getPosWithScreen(QPoint pos)
{
    QPoint dpos;
    QList<ScreenInfo> screensInfo;
    //获取所有实际屏幕的信息
    screensInfo = getScreensInfo();

    for (int i = 0; i < screensInfo.size(); i++) {
        //判断当前点在哪块屏幕上
        // pos 来自 XEventMonitor，为物理坐标；screensInfo 的 width/height 为逻辑坐标，
        // 边界判断时须乘以 scaleFactor 恢复物理值，否则非整数缩放时匹配失效
        if (pos.x() > screensInfo[i].x && pos.x() < screensInfo[i].x + static_cast<int>(screensInfo[i].width * screensInfo[i].scaleFactor) &&
                pos.y() > screensInfo[i].y && pos.y() < screensInfo[i].y + static_cast<int>(screensInfo[i].height * screensInfo[i].scaleFactor)) {

            qInfo() << "screenInfo: " << screensInfo[i].toString();
            // 各屏缩放比可能不同（多屏混缩放），须用所在屏自己的比例换算
            const qreal pixelRatio = screensInfo[i].scaleFactor;
            dpos.setX(static_cast<int>((pos.x() - screensInfo[i].x) / pixelRatio + screensInfo[i].x));
            dpos.setY(static_cast<int>((pos.y() - screensInfo[i].y) / pixelRatio) + + screensInfo[i].y);
        }
    }
    return dpos;
}

//传入屏幕上已经缩放后的点，获取理论上实际的点
QPoint Utils::getPosWithScreenP(QPoint pos)
{
    QPoint dpos;
    QList<ScreenInfo> screensInfo;
    //获取所有实际屏幕的信息
    screensInfo = getScreensInfo();
    for (int i = 0; i < screensInfo.size(); i++) {
        //判断当前点在哪块屏幕上
        // pos 来自 XEventMonitor，为物理坐标；screensInfo 的 width/height 为逻辑坐标，
        // 边界判断时须乘以 scaleFactor 恢复物理值，否则非整数缩放时匹配失效
        if (pos.x() > screensInfo[i].x && pos.x() < screensInfo[i].x + static_cast<int>(screensInfo[i].width * screensInfo[i].scaleFactor) &&
                pos.y() > screensInfo[i].y && pos.y() < screensInfo[i].y + static_cast<int>(screensInfo[i].height * screensInfo[i].scaleFactor)) {

            // 各屏缩放比可能不同（多屏混缩放），须用所在屏自己的比例换算
            const qreal pixelRatio = screensInfo[i].scaleFactor;
            dpos.setX(static_cast<int>((pos.x() - screensInfo[i].x)*pixelRatio + screensInfo[i].x));
            dpos.setY(static_cast<int>((pos.y() - screensInfo[i].y)*pixelRatio + screensInfo[i].y));
        }
    }

    return dpos;
}

QList<Utils::ScreenInfo> Utils::getScreensInfo()
{
    //获取所有实际屏幕的信息
    QList<QScreen *> screenList = qApp->screens();
    QList<ScreenInfo> screensInfo;

    /**
     * 例如：显示器实际屏幕参数为1920*1080记为 t1，调整缩放比例为1.25记为 p,那么此时显示器显示的屏幕大小将变为1536*864记为 t2，
     * 那么此时在 t2的点要变成t1上的点，需要 t2*p = t1,在多屏的情况且需要考虑究竟在那个屏幕上
     * 注意：多屏混缩放（如内屏 150% + 外屏 100%）时各屏 p 不同，
     * 物理尺寸必须按各屏自己的缩放比换算，不能统一乘主屏的比例
     */
    //将实际屏幕信息根据缩放比列转化为理论上屏幕信息
    // 注意：返回的 ScreenInfo.width/height 为逻辑坐标（rect.size），
    // x/y 也为逻辑坐标；调用者若持物理坐标（如 XEventMonitor 上报的点），
    // 须乘以 scaleFactor 恢复物理值后再做边界比较（见 getPosWithScreen/getPosWithScreenP）
    int hTotal = 0;
    for (auto it = screenList.constBegin(); it != screenList.constEnd(); ++it) {
        QRect rect = (*it)->geometry();
        //qDebug() << (*it)->name() << rect;
        const qreal pixelRatio = scaleFactorForScreen(*it);
        ScreenInfo screenInfo;
        screenInfo.x = rect.x(); //屏幕的起始x坐标始终是正确的，不管是否经过缩放
        screenInfo.y = rect.y(); //屏幕的起始y坐标始终是正确的，不管是否经过缩放
        screenInfo.height = rect.height();
        screenInfo.width = rect.width();
        screenInfo.name = (*it)->name();
        screenInfo.scaleFactor = pixelRatio;
        hTotal += screenInfo.height;
        screensInfo.append(screenInfo);
    }
    return screensInfo;
}
