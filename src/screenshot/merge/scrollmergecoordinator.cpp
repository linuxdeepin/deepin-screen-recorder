// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scrollmergecoordinator.h"

#include "../../utils/log.h"
#include "../../utils/pixmergethread.h"

#include <QPixmap>

namespace Dsr::Screenshot {

ScrollMergeCoordinator::ScrollMergeCoordinator(QObject *parent)
    : IScrollImageMerger(parent)
{
}

ScrollMergeCoordinator::~ScrollMergeCoordinator()
{
    cancel();
}

void ScrollMergeCoordinator::start(const QRect &globalSelection)
{
    cancel();
    m_selection = globalSelection.normalized();
    m_lastImage = QImage();
    m_started = false;
    m_finished = false;
    m_merge = new PixMergeThread(this);
    m_merge->setScrollModel(true);
    connect(m_merge, &PixMergeThread::updatePreviewImg,
            this, &IScrollImageMerger::previewUpdated);
    connect(m_merge, &PixMergeThread::merageError,
            this, [this](PixMergeThread::MergeErrorValue state) {
        onMergeError(static_cast<int>(state));
    });
}

bool ScrollMergeCoordinator::addFrame(const ScrollFrame &frame)
{
    if (m_finished || m_selection.isEmpty() || frame.image.isNull()
            || !m_merge) {
        return false;
    }

    const QImage image = frame.image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (!m_lastImage.isNull() && image == m_lastImage)
        return false;

    const QPixmap pixmap = QPixmap::fromImage(image);
    if (pixmap.isNull())
        return false;

    if (!m_started) {
        m_merge->addShotImg(pixmap, PixMergeThread::PictureDirection::Auto);
        m_merge->start();
        m_started = true;
        // PixMergeThread initializes the result canvas with this frame without
        // entering its merge loop, so emit the first preview here. Otherwise
        // the overlay stays empty until the user scrolls far enough to merge a
        // second frame.
        emit previewUpdated(image);
    } else {
        constexpr int MaxPendingFrames = 2;
        if (m_merge->pendingImageCount() >= MaxPendingFrames) {
            qCWarning(dsrApp) << "Dropping oldest scroll frame because merge queue is full";
            m_merge->dropOldestPending();
        }
        m_merge->addShotImg(pixmap, PixMergeThread::PictureDirection::Auto);
    }
    m_lastImage = image;
    return true;
}

QImage ScrollMergeCoordinator::finish()
{
    if (!m_merge || !m_started) {
        m_finished = true;
        return {};
    }

    m_merge->setIsLastImg(true);
    m_merge->stopTask();
    m_merge->wait();
    const QImage result = m_merge->getMerageResult();
    m_finished = true;
    return result;
}

void ScrollMergeCoordinator::cancel()
{
    if (m_merge) {
        m_merge->stopTask();
        m_merge->wait();
        m_merge->deleteLater();
        m_merge = nullptr;
    }
    m_started = false;
    m_finished = true;
    m_lastImage = QImage();
}

void ScrollMergeCoordinator::onMergeError(int state)
{
    switch (static_cast<PixMergeThread::MergeErrorValue>(state)) {
    case PixMergeThread::ReachBottom:
        emit reachedBottom();
        break;
    case PixMergeThread::MaxHeight:
        emit maxHeightReached();
        break;
    case PixMergeThread::InvalidArea:
        emit invalidAreaDetected();
        break;
    case PixMergeThread::RoollingTooFast:
        qCWarning(dsrApp) << "Scroll frame dropped because scrolling was too fast";
        emit quickScrollDetected();
        break;
    case PixMergeThread::Failed:
        emit failed(QStringLiteral("滚动截图拼接失败"));
        break;
    }
}

} // namespace Dsr::Screenshot
