// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ISNAP_HINT_SOURCE_H
#define ISNAP_HINT_SOURCE_H

#include <QObject>
#include <QRect>

namespace Dsr::Screenshot {

class ISnapHintSource : public QObject
{
    Q_OBJECT
public:
    explicit ISnapHintSource(QObject *parent = nullptr) : QObject(parent) {}
    ~ISnapHintSource() override = default;

    virtual bool start(QString *error) = 0;
    virtual void stop() = 0;
    virtual QRect currentRegion() const = 0;

signals:
    void regionChanged(const QRect &globalRegion);
};

} // namespace Dsr::Screenshot

#endif // ISNAP_HINT_SOURCE_H
