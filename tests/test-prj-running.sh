#!/bin/bash

# SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

rm -rf ../build-ut
mkdir ../build-ut
mkdir ../build-ut/report
mkdir ../build-ut/html

cppcheck --enable=all --output-file=../build-ut/cppcheck_deepin-screen-recorder.xml --xml  ../src/
cppcheck --enable=all --output-file=../build-ut/cppcheck_dde-dock-plugins.xml --xml  ../dde-dock-plugins/

./ut_dde_dock_plugins/build_run_ut.sh
# TODO: ut_screen_shot_recorder 需要 3rdparty/libcam 等生产代码完成 Qt6 适配后再启用
#./ut_screen_shot_recorder/build_run_ut.sh

exit 0