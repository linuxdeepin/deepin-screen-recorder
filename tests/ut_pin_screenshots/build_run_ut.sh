#!/bin/bash

# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

export DISPLAY="${DISPLAY:-:0}"
export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-offscreen}"

rm -rf ./ut_pin_screenshots/build-ut
mkdir ./ut_pin_screenshots/build-ut

cd ./ut_pin_screenshots/build-ut
export QT_SELECT=qt6
qmake6 ../
make -j4

mkdir -p html report

executable=ut_pin_screenshots

extract_info="*/pin_screenshots/*"
remove_info="*tests* *build-ut*"

ASAN_OPTIONS="fast_unwind_on_malloc=1"
./${executable} --gtest_output=xml:report/report_${executable}.xml

lcov -d . -c -o ${executable}_coverage.info 2>/dev/null || true
lcov --extract ./${executable}_coverage.info $extract_info --output-file ${executable}_coverage.info 2>/dev/null || true
lcov --remove ./${executable}_coverage.info $remove_info --output-file ${executable}_coverage.info 2>/dev/null || true

# genhtml 和 cp 由主脚本 test-prj-running.sh 统一处理（合并所有模块 .info 后生成一份整体 HTML 报告），
# 此处仅复制 JUnit XML 报告供 CI 统计用例数。
cp ./report/report_${executable}.xml ../../../build-ut/report/ 2>/dev/null || true

exit 0
