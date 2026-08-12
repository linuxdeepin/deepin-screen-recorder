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
# accumulate across processes. Slow/hang-prone tests are skipped for coverage
# but still counted in the report (marked as error).
test_list=$build_dir/${executable}_tests.txt
$build_dir/$executable --gtest_list_tests > $test_list 2>/dev/null
SKIP_TESTS="screenShotShapes screenShot screenRecord scrollShot fullScreenshot fullScreenRecord startRecord stopRecord startAutoScrollShot startManualScrollShot pauseAutoScrollShot continueAutoScrollShot handleManualScrollShot initPadShot delayScreenshot onHelp onViewShortcut fullScreenRecord_screenshotOnly shotCurrentImg shotFullScreen saveTopWindow topWindow initScreenRecorder initScrollShot initBackground setupRegistry waylandwindowinfo"
# 逐用例生成 JUnit XML 报告（供 CI 统计用例数）；崩/hang 用例无 XML，合并时标记为 error。
mkdir -p "$build_dir/report_individual"
: > "$build_dir/ssr_results.tsv"
last_suite=""
while IFS= read -r line; do
  case "$line" in
    " "*)
      tname="${line#"${line%%[![:space:]]*}"}"
      tname="${last_suite}${tname}"
      base="${tname##*.}"
      skip=0
      for s in $SKIP_TESTS; do [ "$base" = "$s" ] && skip=1 && break; done
      if [ "$skip" = "1" ]; then
        printf '%s\t77\n' "$tname" >> "$build_dir/ssr_results.tsv"
        continue
      fi
      safe_name="${tname//[^A-Za-z0-9_]/_}"
      timeout 25 $build_dir/$executable --gtest_filter="$tname" \
        --gtest_output=xml:"report_individual/${safe_name}.xml" >/dev/null 2>&1
      printf '%s\t%d\n' "$tname" "$?" >> "$build_dir/ssr_results.tsv"
      ;;
    *)
      last_suite="$line"
      ;;
  esac
done < $test_list

# 合并逐用例 XML 报告为单一 JUnit XML（供 CI 统计全部用例数）。
# 崩/hang/skip 用例无 XML，按退出码标记为 error，确保用例总数完整。
python3 - "$build_dir/report_individual" "$build_dir/ssr_results.tsv" \
         "$result_report_dir" <<'PYEOF'
import os, sys
from xml.etree import ElementTree as ET

indiv_dir, results_path, out_path = sys.argv[1], sys.argv[2], sys.argv[3]

exit_codes = {}
with open(results_path) as f:
    for line in f:
        parts = line.rstrip('\n').split('\t')
        if len(parts) == 2:
            exit_codes[parts[0]] = int(parts[1])

xml_results = {}
if os.path.isdir(indiv_dir):
    for fn in os.listdir(indiv_dir):
        if not fn.endswith('.xml'):
            continue
        try:
            tree = ET.parse(os.path.join(indiv_dir, fn))
        except ET.ParseError:
            continue
        for tc in tree.iter('testcase'):
            full = f"{tc.get('classname','')}.{tc.get('name','')}"
            fails = tc.findall('failure')
            if fails:
                xml_results[full] = ('fail', tc.get('time','0'), fails[0].get('message',''))
            elif tc.findall('error'):
                xml_results[full] = ('error', tc.get('time','0'), '')
            else:
                xml_results[full] = ('pass', tc.get('time','0'), '')

suites = {}
for full_name, rc in exit_codes.items():
    if '.' not in full_name:
        continue
    suite, tname = full_name.rsplit('.', 1)
    if full_name in xml_results:
        st, tm, msg = xml_results[full_name]
    elif rc == 0:
        st, tm, msg = 'pass', '0', ''
    elif rc == 1:
        st, tm, msg = 'fail', '0', 'gtest assertion failure'
    elif rc == 77:
        st, tm, msg = 'error', '0', 'skipped (known hang/crash)'
    else:
        st, tm, msg = 'error', '0', f'crashed/hang (exit {rc})'
    suites.setdefault(suite, []).append((tname, st, tm, msg))

root = ET.Element('testsuites')
total = sum(len(ts) for ts in suites.values())
total_fail = sum(1 for ts in suites.values() for _, st, _, _ in ts if st == 'fail')
total_err = sum(1 for ts in suites.values() for _, st, _, _ in ts if st == 'error')
root.set('tests', str(total)); root.set('failures', str(total_fail))
root.set('errors', str(total_err)); root.set('disabled', '0')
root.set('name', 'ut_screen_shot_recorder')

for sname, tests in sorted(suites.items()):
    ts = ET.SubElement(root, 'testsuite')
    ts.set('name', sname); ts.set('tests', str(len(tests)))
    ts.set('failures', str(sum(1 for _, st, _, _ in tests if st == 'fail')))
    ts.set('errors', str(sum(1 for _, st, _, _ in tests if st == 'error')))
    ts.set('disabled', '0')
    for tname, st, tm, msg in tests:
        tc = ET.SubElement(ts, 'testcase')
        tc.set('name', tname); tc.set('classname', sname)
        tc.set('status', 'run'); tc.set('time', tm)
        if st == 'fail':
            ET.SubElement(tc, 'failure').set('message', msg)
        elif st == 'error':
            ET.SubElement(tc, 'error').set('message', msg)

ET.ElementTree(root).write(out_path, encoding='UTF-8', xml_declaration=True)
print(f"[INFO] 合并 ssr 报告：{total} 用例，{total_fail} 失败，{total_err} 错误 -> {out_path}")
PYEOF

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