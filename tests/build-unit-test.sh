# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

dsr_build_unit_test() {
    local build_dir="$1" target="$2"
    local root
    root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd) || return 1
    cmake -S "$root" -B "$build_dir" \
        -DCMAKE_BUILD_TYPE=Debug -DDSR_BUILD_TESTS=ON -DDSR_TEST_COVERAGE=ON \
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$build_dir" &&
        cmake --build "$build_dir" --target "$target" --parallel "${DSR_TEST_JOBS:-4}"
}
