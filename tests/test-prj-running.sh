#!/bin/bash

rm -rf ../build-ut
mkdir ../build-ut
mkdir ../build-ut/report
mkdir ../build-ut/html

./ut_dde_dock_plugins/build_run_ut.sh
./ut_screen_shot_recorder/build_run_ut.sh

