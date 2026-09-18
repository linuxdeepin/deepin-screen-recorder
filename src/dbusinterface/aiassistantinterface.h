// Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AIASSISTANTINTERFACE_H
#define AIASSISTANTINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>
#include <QImage>
#include <QBuffer>
#include <QDebug>

class AiAssistantInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    {
        return "com.deepin.copilot";
    }

public:
    /*
    * @param: serviceName QDBusConnection 注册的服务名字
    * @param: ObjectPath QDBusConnection 注册的对象路径
    */
    AiAssistantInterface(const QString &serviceName, const QString &ObjectPath,
                 const QDBusConnection &connection, QObject *parent = nullptr);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    ~AiAssistantInterface();

public Q_SLOTS: // METHODS
    /*
    * @brief: launchAiQuickOCR 启动AI快速OCR功能
    * @param: type 类型 (1=解释, 2=总结, 3=翻译, 4=问问AI)
    * @param: query 查询文本内容
    * @param: pos 位置坐标 QPoint(x, y)
    * @param: isCustom 是否自定义
    * @param: imagePath 图片路径
    * @return: QDBusPendingReply
    * @note: 正确的签名是 'is(ii)bs' (int, string, (int,int), bool, string)
    */
    inline QDBusPendingReply<> launchAiQuickOCR(int type, const QString &query, 
                                                const QPoint &pos,
                                                bool isCustom, const QString &imagePath)
    {
        qDebug() << __FUNCTION__ << "type:" << type << "query:" << query 
                 << "pos:" << pos << "isCustom:" << isCustom << "imagePath:" << imagePath;
        return call(QStringLiteral("launchAiQuickOCR"), type, query, 
                   pos, isCustom, imagePath);
    }

    /*
    * @brief: launchAiQuickOCRWithImage 使用图片数据启动AI快速OCR功能
    * @param: type 类型 (1=解释, 2=总结, 3=翻译, 4=问问AI)
    * @param: image 图片数据
    * @param: imageName 图片名称
    * @return: QDBusPendingReply
    * @note: 使用图片数据而不是文件路径，最后一个参数传递编码后的图片数据
    */
    inline QDBusPendingReply<> launchAiQuickOCRWithImage(int type, const QImage &image, const QString &imageName)
    {
        qDebug() << __FUNCTION__ << "type:" << type << "imageName:" << imageName;
        QByteArray data;
        QBuffer buf(&data);
        if (image.save(&buf, "PNG")) {
            data = qCompress(data, 9);
            data = data.toBase64();
        }
        
        QPoint pos(0, 0);  // 默认位置
        
        // 使用编码后的图片数据作为最后一个字符串参数
        return call(QStringLiteral("launchAiQuickOCR"), type, QString(""), 
                   pos, false, QString::fromUtf8(data));
    }

    /*
    * @brief: launchChatUploadImage 直接上传图片到聊天面板
    * @param: imagePath 图片路径
    * @return: QDBusPendingReply
    */
    inline QDBusPendingReply<> launchChatUploadImage(const QString &imagePath)
    {
        qDebug() << __FUNCTION__ << "imagePath:" << imagePath;
        return call(QStringLiteral("launchChatUploadImage"), imagePath);
    }

Q_SIGNALS: // SIGNALS
};

namespace com {
namespace iflytek {
typedef ::AiAssistantInterface AiAssistant;
}
}
#endif // AIASSISTANTINTERFACE_H
