#!/bin/bash

# SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

export DISPLAY=":0"
export QT_QPA_PLATFORM=

rm -rf ./ut_dde_dock_plugins/ut_record_time/build-ut
mkdir ./ut_dde_dock_plugins/ut_record_time/build-ut

cd ./ut_dde_dock_plugins/ut_record_time/build-ut
pwd
export QT_SELECT=qt6
qmake6 ../
make -j4

workdir=$(pwd)
executable=ut_record_time #可执行程序的文件名

#下面是覆盖率目录操作，一种正向操作，一种逆向操作
extract_info="*/dde-dock-plugins/*" #针对当前目录进行覆盖率操作
remove_info="*/ut_record_time/* *build-ut*"  # 精确匹配测试目录，避免误删路径含 tests 的源码

build_dir=$workdir
result_coverage_dir=$build_dir/html
result_report_dir=$build_dir/report/report_ut_record_time.xml
ASAN_OPTIONS="fast_unwind_on_malloc=1"
$build_dir/$executable --gtest_output=xml:$result_report_dir

lcov -d $build_dir -c -o $build_dir/coverage.info

lcov --extract $build_dir/coverage.info $extract_info --output-file  $build_dir/coverage.info
lcov --remove $build_dir/coverage.info $remove_info --output-file $build_dir/coverage.info

# genhtml 和 cp 由主脚本 test-prj-running.sh 统一处理（合并所有模块 .info 后生成一份整体 HTML 报告），
# 此处仅复制 JUnit XML 报告供 CI 统计用例数。
cp $build_dir/report/report_ut_record_time.xml ../../../../build-ut/report/report_ut_record_time.xml 2>/dev/null || true
# 收集 ASAN 日志（若存在）
cp $build_dir/asan_ut_record_time.log.* ../../../../build-ut/asan_ut_record_time.log 2>/dev/null || true  
exit 0