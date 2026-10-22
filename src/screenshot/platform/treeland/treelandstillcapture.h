// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELAND_STILL_CAPTURE_H
#define TREELAND_STILL_CAPTURE_H

#include <QImage>
#include <QObject>

class ExtCaptureManager;
class QScreen;

namespace Dsr::Screenshot {

class TreelandStillCapture : public QObject
{
    Q_OBJECT
public:
    explicit TreelandStillCapture(QObject *parent = nullptr);
    ~TreelandStillCapture() override;

    QImage capture(QScreen *screen, QString *error);

private:
    bool waitForManager(QString *error);
    static QImage::Format imageFormatForShm(uint32_t format);

    ExtCaptureManager *m_manager = nullptr;
};

} // namespace Dsr::Screenshot

#endif // TREELAND_STILL_CAPTURE_H
