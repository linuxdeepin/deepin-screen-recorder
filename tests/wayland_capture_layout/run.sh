#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
binary=$(mktemp /tmp/wayland-capture-layout-test.XXXXXX)
trap 'rm -f "$binary"' EXIT HUP INT TERM
${CXX:-g++} -std=c++11 -fPIC "$root/tests/wayland_capture_layout/capturelayout_test.cpp" \
    -I"$root/src/waylandrecord" $(pkg-config --cflags --libs Qt5Gui Qt5Core) -o "$binary"
"$binary"
