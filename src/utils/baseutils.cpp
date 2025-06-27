// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "baseutils.h"
#include "log.h"

#include <QPixmap>
#include <QProcess>
#include <QLayoutItem>
#include <QFile>
#include <QApplication>
#include <QDebug>
#include <QIcon>

const QSize START_SIZE = QSize(15, 15);
const QSize RECT_SIZE = QSize(22, 26);
const QSize ARROW_SIZE = QSize(24, 24);
const QSize TEXT_SIZE = QSize(11, 23);
const QSize COLORPEN_SIZE = QSize(25, 25);

QMap<QString, QCursor> BaseUtils::m_shapesCursor = QMap<QString, QCursor>();

QCursor BaseUtils::setCursorShape(QString cursorName, int colorIndex)
{
    qCDebug(dsrApp) << "setCursorShape() called with cursorName:" << cursorName << ", colorIndex:" << colorIndex;

    QString cursorNameString = cursorName;
    if (cursorName == "pen") {
        cursorNameString = QString("%1%2").arg(cursorName).arg(colorIndex);
        qCDebug(dsrApp) << "Adjusted cursorNameString for pen:" << cursorNameString;
    }

    if (BaseUtils::m_shapesCursor.find(cursorNameString) != BaseUtils::m_shapesCursor.end()) {
        qCDebug(dsrApp) << "Using cached cursor for:" << cursorNameString;
        return BaseUtils::m_shapesCursor.value(cursorNameString);
    }

    QCursor customShape = QCursor();
    qreal ration = qApp->devicePixelRatio();
    qCDebug(dsrApp) << "Device pixel ratio:" << ration;

    if (cursorName == "start") {
        qCDebug(dsrApp) << "Creating start cursor";
        QPixmap startPix;
        if (ration <= 1) {
            startPix = QIcon(":/mouse_style/shape/start_mouse.svg").pixmap(START_SIZE);
            customShape = QCursor(startPix, 8, 8);
            qCDebug(dsrApp) << "Loaded start_mouse.svg for ratio <= 1.";
        } else {
            startPix = QIcon(":/mouse_style/shape/start_mouse@2x.svg").pixmap(START_SIZE);
            customShape = QCursor(startPix, int(8 * ration), int(8 * ration));
            qCDebug(dsrApp) << "Loaded start_mouse@2x.svg for ratio > 1.";
        }
    } else if (cursorName == "rotate") {
        qCDebug(dsrApp) << "Creating rotate cursor.";
        QPixmap rotateCursor  = QIcon(":/mouse_style/shape/rotate_mouse.svg").pixmap(ARROW_SIZE);
        rotateCursor.setDevicePixelRatio(ration);

        customShape = QCursor(rotateCursor, int(10 * ration), int(10 * ration));
    } else if (cursorName == "rectangle") {
        qCDebug(dsrApp) << "Creating rectangle cursor.";
        QPixmap rectCursor  = QIcon(":/mouse_style/shape/rect_mouse.svg").pixmap(RECT_SIZE);
        rectCursor.setDevicePixelRatio(ration);

        customShape = QCursor(rectCursor, 0, int(1 * ration));
    } else if (cursorName == "oval") {
        qCDebug(dsrApp) << "Creating oval cursor.";
        QPixmap ovalCursor  = QIcon(":/mouse_style/shape/ellipse_mouse.svg").pixmap(RECT_SIZE);
        ovalCursor.setDevicePixelRatio(ration);

        customShape = QCursor(ovalCursor, 0, int(1 * ration));
    } else if (cursorName == "arrow") {
        qCDebug(dsrApp) << "Creating arrow cursor.";
        QPixmap arrowCursor  = QIcon(":/mouse_style/shape/arrow_mouse.svg").pixmap(ARROW_SIZE);
        arrowCursor.setDevicePixelRatio(ration);

        customShape = QCursor(arrowCursor, int(5 * ration), int(5 * ration));
    } else if (cursorName == "text") {
        qCDebug(dsrApp) << "Creating text cursor.";
        QPixmap textCursor  = QIcon(":/mouse_style/shape/text_mouse.svg").pixmap(TEXT_SIZE);
        textCursor.setDevicePixelRatio(ration);

        customShape = QCursor(textCursor, int(5 * ration), int(5 * ration));
    } else if (cursorName == "pen") {
        qCDebug(dsrApp) << "Creating pen cursor.";
        QPixmap colorPic = QIcon(QString(":/color_pen/color%1.svg").arg(colorIndex)).pixmap(COLORPEN_SIZE);
        colorPic.setDevicePixelRatio(ration);

        customShape = QCursor(colorPic,  int(5 * ration), int(22 * ration));
    } else if (cursorName == "line") {
        qCDebug(dsrApp) << "Creating line cursor.";
        QPixmap lineCursor  = QIcon(":/mouse_style/shape/line_mouse.svg").pixmap(ARROW_SIZE);
        lineCursor.setDevicePixelRatio(ration);

        customShape = QCursor(lineCursor, int(2 * ration), int(9 * ration));
    }

    BaseUtils::m_shapesCursor.insert(cursorNameString, customShape);
    qCDebug(dsrApp) << "Created and cached new cursor for:" << cursorNameString;
    return customShape;
}

int BaseUtils::stringWidth(const QFont &f, const QString &str)
{
    qCDebug(dsrApp) << "stringWidth() called with font and string:" << str;
    QFontMetrics fm(f);
    int width = fm.boundingRect(str).width();
    qCDebug(dsrApp) << "Calculated string width:" << width;
    return width;
}

QColor BaseUtils::colorIndexOf(int index)
{
    qCDebug(dsrApp) << "colorIndexOf() called with index:" << index;
    QList<QColor> colorList;
    colorList.append(QColor("#000000"));
    colorList.append(QColor("#7D7D7D"));
    colorList.append(QColor("#FFFFFF"));
    colorList.append(QColor("#F82A2A"));
    colorList.append(QColor("#FF8100"));
    colorList.append(QColor("#FFF100"));
    colorList.append(QColor("#CDFF00"));
    colorList.append(QColor("#1EE9A8"));
    colorList.append(QColor("#006D06"));
    colorList.append(QColor("#0089F7"));
    colorList.append(QColor("#7600AC"));
    colorList.append(QColor("#0C00A0"));
    return colorList[index];
}

int BaseUtils::colorIndex(QColor color)
{
    qCDebug(dsrApp) << "colorIndex() called with color:" << color;
    QList<QColor> colorList;
    colorList.append(QColor("#000000"));
    colorList.append(QColor("#7D7D7D"));
    colorList.append(QColor("#FFFFFF"));
    colorList.append(QColor("#F82A2A"));
    colorList.append(QColor("#FF8100"));
    colorList.append(QColor("#FFF100"));
    colorList.append(QColor("#CDFF00"));
    colorList.append(QColor("#1EE9A8"));
    colorList.append(QColor("#006D06"));
    colorList.append(QColor("#0089F7"));
    colorList.append(QColor("#7600AC"));
    colorList.append(QColor("#0C00A0"));
    return colorList.indexOf(color);
}

bool BaseUtils::isValidFormat(QString suffix)
{
    qCDebug(dsrApp) << "Checking if format is valid:" << suffix;
    QStringList validFormat;
    validFormat << "bmp" << "jpg" << "jpeg" << "png" << "pbm" << "pgm" << "xbm" << "xpm";
    bool isValid = validFormat.contains(suffix);
    if (!isValid) {
        qCWarning(dsrApp) << "Invalid format detected:" << suffix;
    }
    qCDebug(dsrApp) << "Format" << suffix << "is valid:" << isValid;
    return isValid;
}

bool BaseUtils::isCommandExist(QString command)
{
    qCDebug(dsrApp) << "Checking if command exists:" << command;
    QProcess *proc = new QProcess;
    if (!proc) {
        qCWarning(dsrApp) << "Failed to create QProcess for command check:" << command;
        return false;
    }
    QString cm = QString("which %1\n").arg(command);
    proc->start(cm);
    proc->waitForFinished(1000);
    int ret = proc->exitCode() == 0;
    delete proc;
    proc = nullptr;
    
    if (!ret) {
        qCWarning(dsrApp) << "Command not found:" << command;
    } else {
        qCDebug(dsrApp) << "Command exists:" << command;
    }
    return ret;
}
