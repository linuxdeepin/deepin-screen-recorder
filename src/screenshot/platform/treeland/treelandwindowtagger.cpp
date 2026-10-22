// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandwindowtagger.h"

#include "treelandsnapproxy.h"

namespace Dsr::Screenshot {

TreelandWindowTagger::TreelandWindowTagger(TreelandSnapSession *session, QObject *parent)
    : IWindowTagger(parent)
    , m_session(session)
{
    Q_ASSERT(m_session);
}

bool TreelandWindowTagger::tagMaskWindow(QWindow *window, QString *error)
{
    return m_session->registerWindow(window, error);
}

} // namespace Dsr::Screenshot
