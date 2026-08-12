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

genhtml -o ./html ${executable}_coverage.info 2>/dev/null || true

cp ./report/report_${executable}.xml ../../../build-ut/report/ 2>/dev/null || true
cp ./html/index.html ../../../build-ut/html/cov_${executable}.html 2>/dev/null || true

exit 0
