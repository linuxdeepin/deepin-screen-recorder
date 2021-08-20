/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     He MingYang <hemingyang@uniontech.com>
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

#include "utils.h"
#include "constant.h"
#include <DDialog>
#include <DSysInfo>

#include <QString>
#include <QDir>
#include <QApplication>
#include <QDebug>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QDBusInterface>
#include <QtX11Extras/QX11Info>
#include <QStandardPaths>

#include <X11/extensions/shape.h>
#include <dlfcn.h>

//DWM_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

static const QString WarningDialogService = "com.deepin.dde.WarningDialog";
static const QString WarningDialogPath = "/com/deepin/dde/WarningDialog";
static const QString WarningDialogInterface = "com.deepin.dde.WarningDialog";

bool Utils::is3rdInterfaceStart = false;
bool Utils::isTabletEnvironment = false;
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
/*
void Utils::warnNoComposite()
{
    QDBusInterface iface(WarningDialogService,
                         WarningDialogPath,
                         WarningDialogService);
    iface.call("RaiseWindow");
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
*/
//void Utils::blurRect(DWindowManager *windowManager, int widgetId, QRectF rect)
//{
//    QVector<uint32_t> data;

//    qreal devicePixelRatio = qApp->devicePixelRatio();
//    data << rect.x() * devicePixelRatio << rect.y() * devicePixelRatio << rect.width() * devicePixelRatio << rect.height() * devicePixelRatio << Constant::RECTANGLE_RADIUS << Constant::RECTANGLE_RADIUS;
//    windowManager->setWindowBlur(widgetId, data);
//}

//void Utils::blurRects(DWindowManager *windowManager, int widgetId, QList<QRectF> rects)
//{
//    QVector<uint32_t> data;
//    qreal devicePixelRatio = qApp->devicePixelRatio();
//    foreach (auto rect, rects) {
//        data << rect.x() * devicePixelRatio << rect.y() * devicePixelRatio << rect.width() * devicePixelRatio << rect.height() * devicePixelRatio << Constant::RECTANGLE_RADIUS << Constant::RECTANGLE_RADIUS;
//    }
//    windowManager->setWindowBlur(widgetId, data);
//}

//void Utils::clearBlur(DWindowManager *windowManager, int widgetId)
//{
//    QVector<uint32_t> data;
//    data << 0 << 0 << 0 << 0 << 0 << 0;
//    windowManager->setWindowBlur(widgetId, data);
//}

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
    XRectangle *reponseArea = new XRectangle;
    reponseArea->x = 0;
    reponseArea->y = 0;
    reponseArea->width = 0;
    reponseArea->height = 0;

    XShapeCombineRectangles(QX11Info::display(), static_cast<unsigned long>(wid), ShapeInput, 0, 0, reponseArea, 1, ShapeSet, YXBanded);
    delete reponseArea;
    // dlopen 加载库
    /*
    void *handle = dlopen("libXtst.so", RTLD_LAZY);
    if(!handle){
        qDebug() << "open libXtst.so failure";
        delete reponseArea;
        return;
    }

    void (*XShapeCombineRectangles_handle) (Display*, Window, int, int, int, XRectangle*, int, int, int);
    char *error;

    XShapeCombineRectangles_handle = (void (*) (Display*, Window, int, int, int, XRectangle*, int, int, int))dlsym(handle, "XShapeCombineRectangles");

    if((error = dlerror()) != nullptr){
        qDebug() <<"get libXtst.so function  XShapeCombineRectangles failure";
        delete reponseArea;
        return;
    }
    XShapeCombineRectangles_handle(QX11Info::display(), static_cast<unsigned long>(wid), ShapeInput, 0, 0, reponseArea, 1, ShapeSet, YXBanded);
    dlclose(handle);


    delete reponseArea;
    */
}
/*
QString Utils::getRecordingSaveDirectory()
{
    QDir musicDirectory = QDir(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first());
    QString subDirectory = tr("Recordings");
    musicDirectory.mkdir(subDirectory);
    return musicDirectory.filePath(subDirectory);
}
*/
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
    const int version1040 = 1040;
    if(DSysInfo::isDeepin()) {
        bool correct = false;
        int version = DSysInfo::minorVersion().toInt(&correct);
        if (correct && (version >= version1040)) {
            return true;
        }
    }
    return false;
}
