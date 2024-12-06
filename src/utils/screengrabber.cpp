// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screengrabber.h"

#include "../utils.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDir>
#include <QPixmap>
#include <QScreen>
#include <QDebug>
#include <QGuiApplication>
#include <QApplication>
#include <QDesktopWidget>
#include <QStandardPaths>
#include <QThread>

#define Bool int
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <png.h>

ScreenGrabber::ScreenGrabber(QObject *parent)
    : QObject(parent)
{
}

QPixmap ScreenGrabber::grabEntireDesktop(bool &ok, const QRect &rect, const qreal devicePixelRatio)
{
    ok = true;
    if (Utils::isWaylandMode) {
        int count = 0;
        QRect recordRect {
            static_cast<int>(rect.x() * devicePixelRatio),
            static_cast<int>(rect.y() * devicePixelRatio),
            static_cast<int>(rect.width() * devicePixelRatio),
            static_cast<int>(rect.height() * devicePixelRatio)
        };
        QPixmap res;
        QDBusInterface kwinInterface(QStringLiteral("org.kde.KWin"),
                                     QStringLiteral("/Screenshot"),
                                     QStringLiteral("org.kde.kwin.Screenshot"));
        QDBusReply<QString> reply;
        QString tmpfile = "/tmp/screen.png";
        QFile x11File(tmpfile);
        x11File.remove();
        while (count < 6) {
            reply = kwinInterface.call(QStringLiteral("screenshotFullscreen"));
            res = QPixmap(reply.value());
            if (!res.isNull()) {
                qDebug() << "wayland dbus save image sucess.";
                break;
            }

            // ximage to save
            bool saved = saveDesktop2Png(tmpfile.toStdString().data());
            res = QPixmap(tmpfile);
            if (!res.isNull() && saved) {
                qDebug() << "ximage save image sucess.";
                break;
            }
            count++;
            QThread::msleep(50);
        }
        if (!res.isNull()) {
            QFile dbusResult(reply.value());
            dbusResult.remove();
        } else {
            ok = false;
            qDebug() << __FUNCTION__ << __LINE__ << "Get Pixmap:" << res.size() << "try failed at " << count << "times";
        }
        return res.copy(recordRect);
    }

    QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
    // 在多屏模式下, winId 不是0
    return t_primaryScreen->grabWindow(QApplication::desktop()->winId(), rect.x(), rect.y(), rect.width(), rect.height());
}

bool ScreenGrabber::saveDesktop2Png(const char *filename)
{
    Display *display;
    Window root;
    XWindowAttributes attributes;
    XImage *image;
    int screen_width, screen_height;
    int x = 0, y = 0, width, height;

    bool save_sucess = false;

    // Open the X display
    display = XOpenDisplay(NULL);
    if (!display) {
        qDebug()<<"Unable to open X display\n";
        return save_sucess;
    }

    // Get the root window and screen dimensions
    root = DefaultRootWindow (display);
    XGetWindowAttributes(display, root, &attributes);
    screen_width = attributes.width;
    screen_height = attributes.height;

    // Define the capture area (here capturing the entire screen)
    width = screen_width;
    height = screen_height;

    // Capture the screen image
    image = XGetImage(display, root, x, y, width, height, AllPlanes, ZPixmap);
    if (!image) {
        qDebug()<< "Unable to get image\n";
        XCloseDisplay(display);
        return save_sucess;
    }

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        qDebug()<< "Failed to open file";
        XDestroyImage(image);
        XCloseDisplay(display);
        return save_sucess;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        qDebug()<<  "Failed to create PNG write structure\n";
        fclose(fp);
        XDestroyImage(image);
        XCloseDisplay(display);
        return save_sucess;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        qDebug()<< "Failed to create PNG info structure\n";
        png_destroy_write_struct(&png, nullptr);
        fclose(fp);
        XDestroyImage(image);
        XCloseDisplay(display);
        return save_sucess;
    }

    if (setjmp(png_jmpbuf(png))) {
        fprintf(stderr, "PNG writing error\n");
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        XDestroyImage(image);
        XCloseDisplay(display);
        return save_sucess;
    }

    png_init_io(png, fp);

    // Set the image header information
    png_set_IHDR(png, info, width, height,
                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png, info);

    // Convert the XImage to RGB format and write it to the PNG file
    png_bytep row = (png_bytep)malloc(3 * width);
    for (int y = 0; y < height; ++y) {
        //        XGetImage(display, root, x, y, width, 1, AllPlanes, ZPixmap);

        for (int x = 0; x < width; x++) {
            long pixel = XGetPixel(image, x, y);
            row[x*3] = (pixel & image->red_mask) >> 16;
            row[x*3+1] = (pixel & image->green_mask) >> 8;
            row[x*3+2] = (pixel & image->blue_mask);
        }

        png_write_row(png, row);
    }

    png_write_end(png, nullptr);

    //    QImage qimage = QImage((const uchar *)(image->data), image->width, image->height, image->bytes_per_line, QImage::Format_RGB32);
    //    QPixmap pixmap = QPixmap::fromImage(qimage);

    png_destroy_write_struct(&png, &info);
    fclose(fp);
    free(row);

    save_sucess = true;

    XDestroyImage(image);
    XCloseDisplay(display);

    return save_sucess;
}
