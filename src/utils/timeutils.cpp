// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "timeutils.h"
#include <time.h>
#include <QDateTime>

namespace TimeUtils {

int64_t getMonotonicTimeUs()
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return static_cast<int64_t>(ts.tv_sec) * 1000000LL + static_cast<int64_t>(ts.tv_nsec) / 1000LL;
    }
    // 如果获取单调时间失败，回退到系统时间
    return static_cast<int64_t>(QDateTime::currentMSecsSinceEpoch()) * 1000LL;
}

int64_t getMonotonicTime()
{
    return getMonotonicTimeUs();
}

int64_t getMonotonicTimeMs()
{
    return getMonotonicTimeUs() / 1000LL;
}

int64_t getMonotonicTimeS()
{
    return getMonotonicTimeUs() / 1000000LL;
}

} // namespace TimeUtils
