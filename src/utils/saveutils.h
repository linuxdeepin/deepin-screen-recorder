/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Hou Lei <houlei@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SAVEUTILS_H
#define SAVEUTILS_H

#include <QObject>

enum SaveAction : unsigned int {
    SaveToDesktop,
    SaveToImage,
    AutoSave,
    SaveToSpecificDir,
    SaveToClipboard,
    SaveToAutoClipboard,
    SaveToVideo,    //保存到视频目录
    PadDefaultPath //平板适配采用扩展的思维，尽量可能少的修改桌面代码
};

Q_DECLARE_METATYPE(SaveAction);

#endif // SAVEUTILS_H
