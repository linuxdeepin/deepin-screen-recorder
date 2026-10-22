// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELAND_SNAP_HINT_SOURCE_H
#define TREELAND_SNAP_HINT_SOURCE_H

#include "../../ports/isnaphintsource.h"

namespace Dsr::Screenshot {

class TreelandSnapSession;

class TreelandSnapHintSource : public ISnapHintSource
{
    Q_OBJECT
public:
    explicit TreelandSnapHintSource(TreelandSnapSession *session, QObject *parent = nullptr);

    bool start(QString *error) override;
    void stop() override;
    QRect currentRegion() const override;

private:
    TreelandSnapSession *m_session = nullptr;
};

} // namespace Dsr::Screenshot

#endif // TREELAND_SNAP_HINT_SOURCE_H
