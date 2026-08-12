#!/bin/bash

# SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# 运行全部三个 dock 插件子模块的单元测试与覆盖率。
# 各子模块脚本独立构建/运行/收集覆盖率，任一失败不影响其余。

for mod in ut_record_time ut_shot_start ut_shot_start_record; do
    echo "[INFO] 运行 dock 插件子模块：$mod"
    ./ut_dde_dock_plugins/"$mod"/build_run_ut.sh || echo "[WARN] $mod 运行失败，继续后续模块。"
done

exit 0