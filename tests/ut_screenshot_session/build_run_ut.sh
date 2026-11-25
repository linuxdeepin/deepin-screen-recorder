#!/bin/bash

# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set -e

HERE=$(cd "$(dirname "$0")" && pwd)
source "$HERE/../build-unit-test.sh"
BUILD="$HERE/build-ut"

rm -rf "$BUILD"
mkdir -p "$BUILD/report" "$BUILD/html"

cd "$BUILD"
export QT_QPA_PLATFORM=offscreen
dsr_build_unit_test "$BUILD" ut_screenshot_session

test_status=0
./ut_screenshot_session -o report/report_ut_screenshot_session.xml,junitxml || test_status=$?

lcov -d . -c -o ut_screenshot_session_coverage.info 2>/dev/null || true
lcov --extract ./ut_screenshot_session_coverage.info \
    '*/src/screenshot/*' \
    '*/src/ext-image-capture/framepumpworker.cpp' \
    --output-file ut_screenshot_session_coverage.info 2>/dev/null || true

cp report/report_ut_screenshot_session.xml ../../../build-ut/report/ 2>/dev/null || true
exit "$test_status"
