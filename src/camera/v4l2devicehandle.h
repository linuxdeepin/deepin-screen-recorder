// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef V4L2DEVICEHANDLE_H
#define V4L2DEVICEHANDLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gviewv4l2core.h"

/**
 * @brief Create and remember the current V4L2 device handler.
 */
v4l2_dev_t *create_v4l2_device_handler(const char *device);

/**
 * @brief Return the current V4L2 device handler.
 */
v4l2_dev_t *get_v4l2_device_handler(void);

/**
 * @brief Close and clear the current V4L2 device handler.
 */
void close_v4l2_device_handler(void);

#ifdef __cplusplus
}
#endif

#endif // V4L2DEVICEHANDLE_H
