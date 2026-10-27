// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELAND_STILL_CAPTURE_H
#define TREELAND_STILL_CAPTURE_H

#include <QImage>
#include <QList>
#include <QObject>

class ExtCaptureManager;
class ExtCaptureFrame;
class QScreen;

namespace Dsr::Screenshot {

class TreelandStillCapture : public QObject
{
    Q_OBJECT
public:
    explicit TreelandStillCapture(QObject *parent = nullptr);
    ~TreelandStillCapture() override;

    struct CapturedOutput
    {
        QScreen *screen = nullptr;
        QImage image;
        quint64 timestampNs = 0;
    };

    // Creates every output session first, submits all frame requests without
    // waiting for individual outputs, and then collects the whole batch.
    QList<CapturedOutput> capture(const QList<QScreen *> &screens, QString *error);

private:
    bool waitForManager(QString *error);
    static QImage imageFromFrame(ExtCaptureFrame *frame, QString *error);
    static QImage::Format imageFormatForShm(uint32_t format);

    ExtCaptureManager *m_manager = nullptr;
};

} // namespace Dsr::Screenshot

#endif // TREELAND_STILL_CAPTURE_H
