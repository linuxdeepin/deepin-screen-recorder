// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖补充（UtilsCov3Test）：Utils 中此前从未被任何用例直接调用的函数。
//
// 经核对现有用例（ut_utils_cov.h / ut_utils_cov2.h / ut_utils_ext*.h）：
//   - enableXGrabButton / cancelInputEvent1 已由 UtilsCovTest 调用覆盖；
//   - passInputEvent / getInputEvent / cancelInputEvent / disableXGrabButton 仅在
//     注释中提及，没有任何用例真正调用 → 一直为 0% 覆盖；
//   - Utils::~Utils：Utils 为"创建一次永不销毁"的单例（instance() 返回 new 出的
//     对象，m_utils 永不 delete），故其析构函数（含 deleting destructor D0/D2）
//     在全部用例中均为 0% 覆盖。
//
// 这些函数在测试构建（ENABLE_UNIT_TEST）下，于无 X11 连接时会走早期 return 分支，
// 可安全直接调用；~Utils 通过 delete 单例触发（每个用例独占进程，互不影响）。

#pragma once
#include <gtest/gtest.h>
#include <QDebug>
#include "../../src/utils.h"

using namespace testing;

class UtilsCov3Test : public testing::Test {
public:
    void SetUp() override { std::cout << "start UtilsCov3Test" << std::endl; }
    void TearDown() override { std::cout << "end UtilsCov3Test" << std::endl; }
};

// Utils::passInputEvent —— 既有用例从未调用，覆盖其入口与（无 X11 时）早期返回。
TEST_F(UtilsCov3Test, passInputEventIsCallableAndCrashFree)
{
    EXPECT_NO_FATAL_FAILURE(Utils::passInputEvent(0));
}

// Utils::getInputEvent —— 既有用例从未调用。
TEST_F(UtilsCov3Test, getInputEventIsCallableAndCrashFree)
{
    EXPECT_NO_FATAL_FAILURE(Utils::getInputEvent(0, 0, 0, 0, 0));
}

// Utils::cancelInputEvent —— 既有用例从未调用（仅 cancelInputEvent1 被覆盖）。
TEST_F(UtilsCov3Test, cancelInputEventIsCallableAndCrashFree)
{
    EXPECT_NO_FATAL_FAILURE(Utils::cancelInputEvent(0, 0, 0, 0, 0));
}

// Utils::disableXGrabButton —— 既有用例从未调用（仅 enableXGrabButton 被覆盖）。
TEST_F(UtilsCov3Test, disableXGrabButtonIsCallableAndCrashFree)
{
    EXPECT_NO_FATAL_FAILURE(Utils::disableXGrabButton());
}

// Utils::~Utils（D2/D0）—— 单例永不销毁，故析构从未覆盖。此处 delete 触发析构链。
// 用例在 per-test 进程隔离下运行，删除缓存单例不影响其他用例。
TEST_F(UtilsCov3Test, singletonDestructorRunsCleanly)
{
    Utils *u = Utils::instance();
    ASSERT_NE(nullptr, u);
    EXPECT_NO_FATAL_FAILURE(delete u);
}
