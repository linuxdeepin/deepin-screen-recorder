// Copyright (C) 2020 ~ 2024 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtCore/qglobal.h>

//#if defined(STATIC_LIB)
//#  define LIBDSCREENRECORDERSHARED_EXPORT
//#else

//#endif

#if defined(LIBDSCREEN_RECORDER_LIBRARY)
#  define LIBDSCREENRECORDERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBDSCREENRECORDERSHARED_EXPORT Q_DECL_IMPORT
#endif
