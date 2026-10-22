// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELAND_WINDOW_TAGGER_H
#define TREELAND_WINDOW_TAGGER_H

#include "../../ports/iwindowtagger.h"

namespace Dsr::Screenshot {

class TreelandSnapSession;

class TreelandWindowTagger : public IWindowTagger
{
    Q_OBJECT
public:
    explicit TreelandWindowTagger(TreelandSnapSession *session, QObject *parent = nullptr);

    bool tagMaskWindow(QWindow *window, QString *error) override;

private:
    TreelandSnapSession *m_session = nullptr;
};

} // namespace Dsr::Screenshot

#endif // TREELAND_WINDOW_TAGGER_H
