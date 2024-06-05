// Copyright (C) 2020 ~ 2024 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "imageplatform.h"
#include "videoplatform.h"
#include <memory>

using ImagePlatformPtr = std::unique_ptr<ImagePlatform>;
ImagePlatformPtr loadImagePlatform();

using VideoPlatformPtr = std::unique_ptr<VideoPlatform>;
VideoPlatformPtr loadVideoPlatform();