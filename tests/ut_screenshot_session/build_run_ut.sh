#!/bin/bash

# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set -e

HERE=$(cd "$(dirname "$0")" && pwd)
BUILD="$HERE/build-ut"

rm -rf "$BUILD"
mkdir -p "$BUILD/report" "$BUILD/html"

cd "$BUILD"
export QT_SELECT=qt6
export QT_QPA_PLATFORM=offscreen
qmake6 ../
make -j4

./ut_screenshot_session -o report/report_ut_screenshot_session.xml,junitxml

lcov -d . -c -o ut_screenshot_session_coverage.info 2>/dev/null || true
lcov --extract ./ut_screenshot_session_coverage.info '*/src/screenshot/*' \
    --output-file ut_screenshot_session_coverage.info 2>/dev/null || true

cp report/report_ut_screenshot_session.xml ../../../build-ut/report/ 2>/dev/null || true
