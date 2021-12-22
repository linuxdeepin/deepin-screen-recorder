/*

* Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.

*

* Author:     wangcong <wangcong@uniontech.com>

*

* Maintainer: wangcong <wangcong@uniontech.com>

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

#ifndef PINSCREENSHOTSINTERFACE_H
#define PINSCREENSHOTSINTERFACE_H

#include <QtDBus/QtDBus>
#include <QImage>

class PinScreenShotsInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    {
        return "com.deepin.PinScreenShots";
    }

public:
    /**
    * @param: serviceName QDBusConnection 注册的服务名字
    * @param: ObjectPath QDBusConnection 注册的对象路径
    */
    PinScreenShotsInterface(const QString &serviceName, const QString &ObjectPath,
                            const QDBusConnection &connection, QObject *parent = nullptr);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    ~PinScreenShotsInterface();

public Q_SLOTS: // METHODS
    /**
    * @bref:openFile 通过路径打开图片文件
    * @param: filePath 图片的路径
    * @return: QDBusPendingReply
    */
    inline QDBusPendingReply<> openFile(const QString &filePath)
    {
        qDebug() << "PinScreenShotsInterface: " << __FUNCTION__;
        return call(QStringLiteral("openFile"), filePath);
    }
    /**
    * @bref:openImages
    * @param: image 图片
    * @return: QDBusPendingReply
    * @note:
    */
    inline QDBusPendingReply<> openImage(const QImage &image)
    {
        qDebug() << "PinScreenShotsInterface: "  << __FUNCTION__;
        QByteArray data;
        QBuffer buf(&data);
        if (image.save(&buf, "PNG")) {
            data = qCompress(data, 9);
            data = data.toBase64();
        }
        return call(QStringLiteral("openImage"), QVariant::fromValue(data));
    }

    /*
    * @bref:openImageAndName
    * @param: image 图片
    * @param: imageName 图片名称
    * @return: QDBusPendingReply
    * @note:
    */
    inline QDBusPendingReply<> openImageAndName(const QImage &image, const QString &imageName)
    {
        qDebug() << "PinScreenShotsInterface: "  << __FUNCTION__;
        QByteArray data;
        QBuffer buf(&data);
        if (image.save(&buf, "PNG")) {
            data = qCompress(data, 9);
            data = data.toBase64();
        }
        return call(QStringLiteral("openImageAndName"), QVariant::fromValue(data), imageName);
    }

Q_SIGNALS: // SIGNALS
};
namespace com {
namespace deepin {
typedef ::PinScreenShotsInterface PinScreenShots;
}
}
#endif // PINSCREENSHOTSINTERFACE_H
