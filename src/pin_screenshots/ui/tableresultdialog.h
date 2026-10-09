// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABLERESULTDIALOG_H
#define TABLERESULTDIALOG_H

#include <DBlurEffectWidget>
#include <DDialog>
#include <DWidget>

#include <QImage>
#include <QString>

DWIDGET_USE_NAMESPACE

/**
 * @brief 表格识别成功弹窗
 *
 * 搭法完全对照 DTK 自带例子 examples/collections/dialogexample.cpp 里
 * 「开始还原」按钮弹出来的那个对话框（标准 DDialog）：
 *   setIcon()      —— 左上角应用图标
 *   setTitle()     —— 居中标题
 *   addContent()   —— 我们特有的内容（识别成功插图 + 文案）
 *   addButton()    —— 底部按钮行
 * 图标 / 居中标题 / 右上角关闭按钮 / 圆角 / 投影 / 按钮行全部由 DDialog 负责，
 * 我们只往里面塞这一步特有的控件。
 */
class TableResultDialog : public DDialog
{
    Q_OBJECT
public:
    explicit TableResultDialog(const QString &html, const QImage &backdrop = QImage(),
                              DWidget *parent = nullptr);
    ~TableResultDialog() override;

    /// 贴图窗口截图：作为毛玻璃的模糊来源（DDialog 默认走窗管，贴图窗绕过了窗管）
    void setBackdropImage(const QImage &image);

    /// 重新识别成功时更新结果内容，避免复用同一个弹窗时剪贴板残留上一次的表格
    void setResultHtml(const QString &html);

    /// 定位到贴图窗口中间并显示
    void showOverlay();

private:
    void copyToClipboard() const;
    void setupFrostedBackground();
    void applyThemeColors();
    void applyCloseButtonMask();

    QString m_html;
    QImage m_backdrop;
    Dtk::Widget::DBlurEffectGroup *m_blurGroup = nullptr;
};

#endif // TABLERESULTDIALOG_H
