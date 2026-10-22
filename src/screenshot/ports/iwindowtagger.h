// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IWINDOW_TAGGER_H
#define IWINDOW_TAGGER_H

#include <QObject>
#include <QString>

class QWindow;

namespace Dsr::Screenshot {

class IWindowTagger : public QObject
{
    Q_OBJECT
public:
    explicit IWindowTagger(QObject *parent = nullptr) : QObject(parent) {}
    ~IWindowTagger() override = default;

    virtual bool tagMaskWindow(QWindow *window, QString *error) = 0;
};

} // namespace Dsr::Screenshot

#endif // IWINDOW_TAGGER_H
