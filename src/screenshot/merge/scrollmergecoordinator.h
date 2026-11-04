// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCROLL_MERGE_COORDINATOR_H
#define SCROLL_MERGE_COORDINATOR_H

#include "../ports/iscrollmerger.h"

#include <QImage>

class PixMergeThread;

namespace Dsr::Screenshot {

class ScrollMergeCoordinator final : public IScrollImageMerger
{
    Q_OBJECT
public:
    explicit ScrollMergeCoordinator(QObject *parent = nullptr);
    ~ScrollMergeCoordinator() override;

    void start(const QRect &globalSelection) override;
    bool addFrame(const ScrollFrame &frame) override;
    QImage finish() override;
    void cancel() override;

private:
    void onMergeError(int state);

    PixMergeThread *m_merge = nullptr;
    QRect m_selection;
    QImage m_lastImage;
    bool m_started = false;
    bool m_finished = false;
};

} // namespace Dsr::Screenshot

#endif // SCROLL_MERGE_COORDINATOR_H
