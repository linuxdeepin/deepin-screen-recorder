// Copyright (C) 2020 ~ 2024 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "cmdlineoptions.h"

struct CmdLineOptionsSingleton {
    CmdLineOptions self;
};

Q_GLOBAL_STATIC(CmdLineOptionsSingleton, privateCmdLineOptionsSelf)

CmdLineOptions *CmdLineOptions::self()
{
    return &privateCmdLineOptionsSelf()->self;
}

const QCommandLineOption CmdLineOptions::getOption(const Option &option)
{
    QCommandLineOption cmdLineOption("");
    if (option >= 0 && option < Option::TotalOptions) {
        cmdLineOption = allOptions[option];
    }
    return cmdLineOption;
}
