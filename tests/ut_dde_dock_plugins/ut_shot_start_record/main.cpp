// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QDebug>
#include <QApplication>

#include <sanitizer/asan_interface.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qDebug() << "start ShotStartRecrodPlugin test cases ..............";
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
#ifdef ENABLE_TSAN_TOOL
    __sanitizer_set_report_path("./asan_ut_shot_start_record.log");
#endif
    qDebug() << "end ShotStartRecrodPlugin test cases ..............";
    return ret;
}
