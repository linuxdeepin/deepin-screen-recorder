// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "v4l2devicehandle.h"
#include "cameraconfig.h"
#include "gviewv4l2core.h"

#include <stdlib.h>
#include <string.h>

static v4l2_dev_t *my_vd = NULL;

v4l2_dev_t *create_v4l2_device_handler(const char *device)
{
    my_vd = v4l2core_init_dev(device);
    return my_vd;
}

v4l2_dev_t *get_v4l2_device_handler(void)
{
    return my_vd;
}

void close_v4l2_device_handler(void)
{
    if (my_vd) {
        v4l2core_close_dev(my_vd);
        my_vd = NULL;
    }
}

/*
 * libcam configuration parsing expects the following setters. They were
 * previously provided by the unused libcam GUI module.
 */
void set_device_location(const char *name)
{
    config_t *config = config_get();
    free(config->device_location);
    config->device_location = name ? strdup(name) : NULL;
}

void set_device_name(const char *name)
{
    config_t *config = config_get();
    free(config->device_name);
    config->device_name = name ? strdup(name) : NULL;
}

void set_profile_name(const char *name)
{
    config_t *config = config_get();
    free(config->profile_name);
    config->profile_name = name ? strdup(name) : NULL;
}

void set_profile_path(const char *path)
{
    config_t *config = config_get();
    free(config->profile_path);
    config->profile_path = path ? strdup(path) : NULL;
}
