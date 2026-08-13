#!/bin/bash

# SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

export DISPLAY=":0"
export QT_QPA_PLATFORM=

rm -rf ./ut_dde_dock_plugins/ut_shot_start_record/build-ut
mkdir ./ut_dde_dock_plugins/ut_shot_start_record/build-ut

cd ./ut_dde_dock_plugins/ut_shot_start_record/build-ut

mkdir -p html
mkdir -p report

export QT_SELECT=qt6
qmake6 ../
make -j4

executable=ut_shot_start_record #可执行程序的文件名

#下面是覆盖率目录操作，一种正向操作，一种逆向操作
extract_info="*/dde-dock-plugins/*" #针对当前目录进行覆盖率操作
remove_info="*tests* *build-ut*" #排除当前目录进行覆盖率操作

ASAN_OPTIONS="fast_unwind_on_malloc=1"
./${executable} --gtest_output=xml:report/report_${executable}.xml

lcov -d . -c -o ${executable}_coverage.info

lcov --extract ./${executable}_coverage.info ${extract_info} --output-file  ${executable}_coverage.info

lcov --list-full-path -e ${executable}_coverage.info –o ./coverage-stripped.info

# genhtml 和 cp 由主脚本 test-prj-running.sh 统一处理（合并所有模块 .info 后生成一份整体 HTML 报告），
# 此处仅复制 JUnit XML 报告供 CI 统计用例数。
cp -r ./report/ ../../../../build-ut 2>/dev/null || true
# 收集 ASAN 日志（若存在）
cp ./asan_${executable}.log ../../../../build-ut 2>/dev/null || true

exit 0

