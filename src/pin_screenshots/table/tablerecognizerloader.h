// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABLERECOGNIZERLOADER_H
#define TABLERECOGNIZERLOADER_H

#include <QImage>
#include <QObject>
#include <QString>

// 表格识别能力由 dtkmultimedia 的 dtk6tablerecognizer 提供，走常规链接
// （见 pin_screenshots.pro 的 PKGCONFIG += dtk6tablerecognizer）。
// 头文件由构建依赖 libdtk6tablerecognizer-dev 提供，缺失时构建直接失败；
// 运行期依赖由打包时的 ${shlibs:Depends} 自动生成。
#include <dtablerecognizer.h>

/**
 * @brief 表格识别能力（dtkmultimedia）的封装
 *
 * 直接持有 Dtk::TableRecognizer::DTableRecognizer；识别结束后把结果和库给出的
 * 错误码（DTableResult::error）交给展示层，展示层只依据错误码选择提示文案，
 * 不对 errorMessage 做任何字符串匹配。
 */
class TableRecognizerLoader : public QObject
{
    Q_OBJECT
public:
    explicit TableRecognizerLoader(QObject *parent = nullptr);

    /**
     * @brief 异步识别表格
     * @param image 待识别图片
     * @param timeoutMs 识别超时时间（毫秒）
     * @return 是否成功启动识别流程
     */
    bool recognize(const QImage &image, int timeoutMs = 25000);

Q_SIGNALS:
    void recognitionFinished(bool success, const QString &html,
                             Dtk::TableRecognizer::TableError error);

private:
    Dtk::TableRecognizer::DTableRecognizer *m_recognizer = nullptr;
};

#endif // TABLERECOGNIZERLOADER_H
