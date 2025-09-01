// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIMEUTILS_H
#define TIMEUTILS_H

#include <cstdint>

namespace TimeUtils {

/**
 * @brief 获取单调时间戳（微秒），避免系统时间调整导致的时间跳变
 * 
 * 使用Linux的CLOCK_MONOTONIC时钟，该时钟从系统启动开始计时，
 * 不会因为系统时间调整（如NTP同步、手动修改时间）而跳变。
 * 返回值与av_gettime()兼容，都是微秒级时间戳。
 * 
 * @return int64_t 微秒级时间戳
 */
int64_t getMonotonicTimeUs();

/**
 * @brief 获取单调时间戳（微秒），与av_gettime()兼容
 * 
 * 这是getMonotonicTimeUs()的别名，为了与FFmpeg的av_gettime()保持一致的命名。
 * 
 * @return int64_t 微秒级时间戳
 */
int64_t getMonotonicTime();

/**
 * @brief 获取单调时间戳（毫秒），避免系统时间调整导致的时间跳变
 * 
 * @return int64_t 毫秒级时间戳
 */
int64_t getMonotonicTimeMs();

/**
 * @brief 获取单调时间戳（秒），避免系统时间调整导致的时间跳变
 * 
 * @return int64_t 秒级时间戳
 */
int64_t getMonotonicTimeS();

} // namespace TimeUtils

#endif // TIMEUTILS_H
