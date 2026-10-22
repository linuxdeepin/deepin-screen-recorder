// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandsnaphintsource.h"

#include "treelandsnapproxy.h"

namespace Dsr::Screenshot {

TreelandSnapHintSource::TreelandSnapHintSource(TreelandSnapSession *session, QObject *parent)
    : ISnapHintSource(parent)
    , m_session(session)
{
    Q_ASSERT(m_session);
    connect(m_session, &TreelandSnapSession::regionChanged,
            this, &ISnapHintSource::regionChanged);
}

bool TreelandSnapHintSource::start(QString *error)
{
    return m_session->start(error);
}

void TreelandSnapHintSource::stop()
{
    m_session->stop();
}

QRect TreelandSnapHintSource::currentRegion() const
{
    return m_session->region();
}

} // namespace Dsr::Screenshot
