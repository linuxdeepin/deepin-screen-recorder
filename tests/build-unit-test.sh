# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

dsr_build_unit_test() {
    local build_dir="$1" target="$2"
    local root
    root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd) || return 1
    cmake -S "$root" -B "$build_dir" \
        -DCMAKE_BUILD_TYPE=Debug -DDSR_BUILD_TESTS=ON -DDSR_TEST_COVERAGE=ON \
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$build_dir" &&
    # 注：不要用 cmake --build。CI 覆盖率流水线会注入 cmake 包装器，给每条
    # cmake 命令追加 -DCMAKE_COVERAGE_ARG=...；cmake --build 不识别该参数
    # 直接报 "Unknown argument" 退出，导致所有单测目标编译失败、覆盖率为空。
    # 默认生成器是 Unix Makefiles，直接调 make 可规避。
    make -C "$build_dir" "$target" -j"${DSR_TEST_JOBS:-4}"
}
