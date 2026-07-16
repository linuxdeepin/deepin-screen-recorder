#!/bin/bash

# SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# 运行单元测试 + 聚合覆盖率，目标：报告覆盖 src/ 下全部源码（真实覆盖率）。
#
# 关键设计：
#   - ut_screen_shot_recorder：逐用例隔离运行（每个用例独立进程 + 单用例超时）。
#     原因：ssr 有若干用例会 ASan abort 或 hang；单次全量跑一崩就丢全部 .gcda
#     （abort 跳过 gcov flush）。逐用例隔离则：正常退出的 flush 自己那份，
#     崩/hang 的只丢自己，跨用例累积 = 并集覆盖率。
#   - lcov：ssr 零基线(--capture --initial, 全 0) + ssr 运行时(--capture, 真实)
#     + recordtime 真实，合并；未执行文件留 0%，已执行按真实。
#
# 注意：逐用例运行较慢（~10-20 分钟），可用 SSR_TEST_TIMEOUT 调单用例超时。

HERE=$(cd "$(dirname "$0")" && pwd)        # .../tests
# 主进程切到 tests/：脚本后续 rm -rf 会删除 build-ut，若调用方 cwd 恰在其中
# (后台运行时常见，cwd 会持久化在 build-ut)，会使本进程 cwd 失效，进而令
# --gtest_list_tests 等子进程在 dxcb 平台初始化时 SEGV → 收集到 0 个用例。
cd "$HERE" || exit 1
ROOT=$(cd "$HERE/.." && pwd)               # 仓库根
OUT="$ROOT/build-ut"
SSR_TEST_TIMEOUT="${SSR_TEST_TIMEOUT:-75}"

# 确保有 DISPLAY：后台/CI 环境可能未继承 DISPLAY，而 native 平台的
# QApplication 构造依赖 X，否则 --gtest_list_tests 即崩、收集到 0 个用例。
export DISPLAY="${DISPLAY:-:0}"

rm -rf "$OUT"
mkdir -p "$OUT/report" "$OUT/html"

# 静态分析（cppcheck 未安装时跳过）
if command -v cppcheck >/dev/null 2>&1; then
    cppcheck --enable=all --output-file="$OUT/cppcheck_deepin-screen-recorder.xml" --xml "$ROOT/src/"
    cppcheck --enable=all --output-file="$OUT/cppcheck_dde-dock-plugins.xml" --xml "$ROOT/src/dde-dock-plugins/"
else
    echo "[WARN] cppcheck 未安装，跳过静态分析。"
fi

# 1) 编译 ut_screen_shot_recorder，为 src/ 主体生成 .gcno
SSR="$HERE/ut_screen_shot_recorder"
SSR_DIR="$SSR/build-ut"
rm -rf "$SSR_DIR"
mkdir -p "$SSR_DIR"
(
    cd "$SSR_DIR" || exit 1
    export QT_SELECT=qt6
    qmake6 ../
    make -j4
) || echo "[WARN] ut_screen_shot_recorder 编译存在错误，将用已生成的 .gcno 继续。"

# 1.5) 逐用例隔离运行 ssr（规避崩/hang 丢覆盖；.gcda 跨用例累积）
SSR_BIN="$SSR_DIR/ut_screen_shot_recorder"
if [ -x "$SSR_BIN" ]; then
    find "$SSR_DIR" -name "*.gcda" -delete 2>/dev/null
    mapfile -t SSR_TESTS < <("$SSR_BIN" --gtest_list_tests 2>/dev/null | awk '
        /^[A-Za-z0-9_]+.*\.$/{s=$0; sub(/\.$/, "", s)}
        /^  [A-Za-z0-9_]+/{gsub(/^ +/, ""); print s"."$0}')
    ssr_ok=0; ssr_bad=0
    # native 平台运行（环境有真实屏幕+摄像头）：offscreen 会让 MainWindow/
    # ShapesWidget 的 paint/cursor/事件路径、camera、screengrab 跑不全，覆盖偏低。
    # ASAN 已关闭(TSAN_TOOL_ENABLE=false)，native 不会因 UAF abort；逐用例隔离
    # 保证偶发崩/hang 只丢自己那份 gcda。
    for t in "${SSR_TESTS[@]}"; do
        ( cd "$SSR_DIR" && DISPLAY=:0 QT_LOGGING_RULES="*=false" \
          ASAN_OPTIONS=fast_unwind_on_malloc=1:detect_leaks=0 \
          timeout "$SSR_TEST_TIMEOUT" ./ut_screen_shot_recorder --gtest_filter="$t" >/dev/null 2>&1 )
        case $? in 0|1) ssr_ok=$((ssr_ok+1));; *) ssr_bad=$((ssr_bad+1));; esac
    done
    echo "[INFO] ssr 逐用例：$ssr_ok 正常退出，$ssr_bad 崩/hang 跳过（共 ${#SSR_TESTS[@]} 个）。"
else
    echo "[WARN] ssr 二进制不存在，跳过其运行（仅 recordtime 覆盖）。"
fi

# 2) 编译 + 运行 ut_record_time（真实覆盖率）
"$HERE/ut_dde_dock_plugins/build_run_ut.sh"

# 3) 聚合覆盖率（ssr 基线 + ssr 运行时 + recordtime）
RT_DIR="$HERE/ut_dde_dock_plugins/ut_record_time/build-ut"

lcov --capture --initial -d "$SSR_DIR" -o "$OUT/ssr_base.info" 2>/dev/null \
    || echo "[WARN] 未能生成 ssr 零覆盖基线。"
lcov --capture -d "$SSR_DIR" -o "$OUT/ssr_rt.info" 2>/dev/null \
    || echo "[WARN] 未能捕获 ssr 运行时覆盖率。"

# ssr 真实 = 基线(0) + 运行时(真实)
if [ -f "$OUT/ssr_base.info" ] && [ -f "$OUT/ssr_rt.info" ]; then
    lcov --add-tracefile "$OUT/ssr_base.info" --add-tracefile "$OUT/ssr_rt.info" -o "$OUT/ssr_real.info" 2>/dev/null
elif [ -f "$OUT/ssr_rt.info" ]; then
    cp "$OUT/ssr_rt.info" "$OUT/ssr_real.info"
elif [ -f "$OUT/ssr_base.info" ]; then
    cp "$OUT/ssr_base.info" "$OUT/ssr_real.info"
fi

# recordtime 真实覆盖率
if [ -f "$RT_DIR/coverage.info" ]; then
    cp "$RT_DIR/coverage.info" "$OUT/rt.info"
else
    lcov --capture -d "$RT_DIR" -o "$OUT/rt.info" 2>/dev/null || true
fi

# 合并 ssr 真实 + recordtime
if [ -f "$OUT/ssr_real.info" ] && [ -f "$OUT/rt.info" ]; then
    lcov --add-tracefile "$OUT/ssr_real.info" --add-tracefile "$OUT/rt.info" -o "$OUT/coverage-merged.info" 2>/dev/null
elif [ -f "$OUT/ssr_real.info" ]; then
    cp "$OUT/ssr_real.info" "$OUT/coverage-merged.info"
elif [ -f "$OUT/rt.info" ]; then
    cp "$OUT/rt.info" "$OUT/coverage-merged.info"
else
    echo "[ERROR] 无可用覆盖率数据。" >&2
    exit 1
fi

# 只保留 src/ 下源码（剔除 3rdparty/build 产物）
lcov --extract "$OUT/coverage-merged.info" '*/src/*' -o "$OUT/coverage.info"

# 剔除 Wayland 生成代码：QtWayland 生成的协议绑定(qwayland-*、protocols/)与
# capture.cpp(Treeland 协议胶水)依赖真实 Wayland 合成器绑定，无头单测环境
# 结构上无法覆盖(QWaylandClientExtension 构造/方法会 SEGV)。这些代码不计入
# 单测覆盖率分母，使指标反映真正可单测的代码。
lcov --remove "$OUT/coverage.info" \
    '*/src/protocols/*' \
    '*/src/qwayland-*' \
    '*/src/capture.cpp' \
    '*/src/*-protocol.c' \
    '*/src/*-protocol.h' \
    '*/src/main.cpp' \
    '*/src/waylandrecord/*' \
    '*/src/utils/waylandmousesimulator.cpp' \
    '*/src/utils/waylandscrollmonitor.cpp' \
    '*/src/camera/LPF_V4L2.c' \
    '*/src/camera/majorimageprocessingthread.cpp' \
    '*/src/dbusservice/dbusscreenshotservice.cpp' \
    -o "$OUT/coverage.info"

genhtml -o "$OUT/html/src-overview" "$OUT/coverage.info"

echo "================ 覆盖率汇总（src/，已剔除 Wayland 生成代码）================"
lcov --summary "$OUT/coverage.info"
echo "（已剔除：protocols/、qwayland-*、capture.cpp、*-protocol.*、main.cpp、waylandrecord/、waylandmousesimulator/waylandscrollmonitor、LPF_V4L2/majorimageprocessingthread、dbusscreenshotservice —— 需真实 Wayland 合成器/X11/摄像头设备，无法单测）"
echo "HTML 报告：$OUT/html/src-overview/index.html"

exit 0
