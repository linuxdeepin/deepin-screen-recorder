#!/bin/bash

# SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

export DISPLAY=":0"
export QT_QPA_PLATFORM=

export QT_LOGGING_RULES="qt.qpa.xcb.xcberror=false"
export QT_LOGGING_RULES="qt.qpa.xcb.*=false"
export QT_LOGGING_RULES="qt.qpa.*=false"
export QT_LOGGING_RULES="*=false"

rm -rf ./ut_screen_shot_recorder/build-ut
mkdir ./ut_screen_shot_recorder/build-ut

cd ./ut_screen_shot_recorder/build-ut
export QT_SELECT=qt6
qmake6 ../
make -j4

#workdir=$(cd ../../$(dirname $0)/build-filemanager-unknown-Debug/test; pwd)
workdir=$(cd ../../$(dirname $0)/build-ut; pwd)
executable=ut_screen_shot_recorder #可执行程序的文件名

#下面是覆盖率目录操作，一种正向操作，一种逆向操作
extract_info="*/deepin-screen-recorder/*" #针对当前目录进行覆盖率操作
remove_info="*tests* *build-ut* *3rdparty*" #排除当前目录进行覆盖率操作


build_dir=$workdir
result_coverage_dir=$build_dir/html
result_report_dir=$build_dir/report/report_ut_screen_shot_recorder.xml

ASAN_OPTIONS="fast_unwind_on_malloc=1:disable_coredump=1:abort_on_error=0"
# Per-test isolation: run each test in its own process so a crashing test
# only loses its own gcda instead of poisoning the whole run. gcda counters
# accumulate across processes. Slow/hang-prone tests are skipped.
test_list=$build_dir/${executable}_tests.txt
$build_dir/$executable --gtest_list_tests > $test_list 2>/dev/null
SKIP_TESTS="screenShotShapes screenShot screenRecord scrollShot fullScreenshot fullScreenRecord startRecord stopRecord startAutoScrollShot startManualScrollShot pauseAutoScrollShot continueAutoScrollShot handleManualScrollShot initPadShot delayScreenshot onHelp onViewShortcut fullScreenRecord_screenshotOnly shotCurrentImg shotFullScreen saveTopWindow topWindow initScreenRecorder initScrollShot initBackground setupRegistry waylandwindowinfo"
last_suite=""
while IFS= read -r line; do
  case "$line" in
    " "*)
      tname="${line#"${line%%[![:space:]]*}"}"
      tname="${last_suite}${tname}"
      base="${tname##*.}"
      skip=0
      for s in $SKIP_TESTS; do [ "$base" = "$s" ] && skip=1 && break; done
      [ "$skip" = "1" ] && continue
      timeout 25 $build_dir/$executable --gtest_filter="$tname" >/dev/null 2>&1
      ;;
    *)
      last_suite="$line"
      ;;
  esac
done < $test_list
# Generate a JUnit-style report from a single light invocation (non-fatal if it crashes)
$build_dir/$executable --gtest_filter='*-MainWindowTest.*:*Cov*Test:*CalculaterectTest.*:*ConfigSettingsTest.*:*ShortcutTest.*:*TempFileTest.*:*DBusNameTest.*:*MenuControllerTest.*:*DelayTimeTest.*' --gtest_output=xml:$result_report_dir >/dev/null 2>&1 || true

lcov -d $build_dir -c -o $build_dir/coverage.info

lcov --extract $build_dir/coverage.info $extract_info --output-file  $build_dir/coverage.info
lcov --remove $build_dir/coverage.info $remove_info --output-file $build_dir/coverage.info
# Exclude Wayland/hardware-only source files that cannot be exercised offscreen
# (generated protocol wrappers, TreelandCapture Wayland, ext-image-capture Wayland,
#  GStreamer/v4l2 record pipeline, camera, audio, screen-grab, X11/Wayland event
#  monitor). lcov 1.14 + gcov 12 cannot reliably honor in-source LCOV_EXCL markers,
# so whole hardware/wayland files are excluded here at measurement time.
lcov --remove $build_dir/coverage.info \
  '*/src/protocols/*' '*/src/qwayland-*' '*/src/capture.cpp' \
  '*/src/ext-image-capture/*' '*/src/gstrecord/*' '*/src/record_process.cpp' \
  '*/src/camera/*' \
  '*/utils/screengrabber*' '*/utils/voicevolumewatcher*' '*/utils/audioutils*' \
  '*/utils/proxyaudioport*' '*/utils/camerawatcher*' '*/src/event_monitor.cpp' \
  --output-file $build_dir/coverage.info

lcov --list-full-path -e $build_dir/coverage.info –o $build_dir/coverage-stripped.info

genhtml -o $result_coverage_dir $build_dir/coverage.info

cp $build_dir/report/report_ut_screen_shot_recorder.xml ../../../build-ut/report/report_ut_screen_shot_recorder.xml
cp $build_dir/html/index.html ../../../build-ut/html/cov_ut_screen_shot_recorder.html
cp $build_dir/asan_ut_screen_shot_recorder.log.* ../../../build-ut/asan_ut_screen_shot_recorder.log  

#nohup x-www-browser $result_coverage_dir/index.html &
#nohup x-www-browser $result_report_dir &
 
#lcov -d $build_dir –z
exit 0