// Copyright (C) 2020 ~ 2024 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "platformloader.h"
#include "defaultplatform.h"

#include <QDebug>

ImagePlatformPtr getForcedImagePlatform()
{
    return nullptr;
}

ImagePlatformPtr loadImagePlatform()
{
    return std::make_unique<ImageDefaultPlatform>();
}

VideoPlatformPtr getForcedVideoPlatform()
{
    return nullptr;
}

VideoPlatformPtr loadVideoPlatform()
{
    return std::make_unique<VideoDefaultPlatform>();
}
