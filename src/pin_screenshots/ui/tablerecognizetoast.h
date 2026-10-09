// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABLERECOGNIZETOAST_H
#define TABLERECOGNIZETOAST_H

#include <DFloatingMessage>
#include <DPushButton>
#include <DWidget>

#include <QColor>
#include <QImage>
#include <QPointer>

DWIDGET_USE_NAMESPACE

/**
 * @brief 表格识别提示条（Toast）
 *
 * 直接使用 DTK 现成的通知控件 DFloatingMessage（内部自带毛玻璃背景
 * DBlurEffectWidget、圆角、阴影、图标、文字和自定义控件位），不再自行绘制。
 * 按设计稿 symbol“DTK/通知与提醒/应用通知/提醒”覆盖参数：
 * 高 42、圆角 12、背景模糊 radius 30、fill rgba(247,247,247,0.60)、
 * shadow blur 20 / offsetY 8 / rgba(0,0,0,0.10)。
 */
class TableRecognizeToast : public DFloatingMessage
{
    Q_OBJECT
public:
    enum Status {
        Error = 0,
        Info
    };

    explicit TableRecognizeToast(const QString &message,
                                 const QString &actionText = QString(),
                                 bool closable = false,
                                 Status status = Error,
                                 DWidget *parent = nullptr);
    ~TableRecognizeToast() override;

    /**
     * @brief 贴在宿主窗口底部居中显示
     *
     * 作为宿主窗口的子控件浮层：这样 DFloatingMessage 内部的
     * DBlurEffectWidget 才能从宿主窗口的绘制结果里取到真实的背景做模糊。
     */
    void popupIn(QWidget *host);

    /// 兼容旧接口：DTK 的模糊背景直接取宿主窗口内容，不再需要外部截图
    void setBackdropImage(const QImage &image);

Q_SIGNALS:
    void actionTriggered();
    void closed();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    /// 浅色/深色切换时刷新填充与文字颜色
    void applyThemeColors();
    void updateLayoutPosition();
    void updateStatusIcon();

    DPushButton *m_actionButton = nullptr;
    QPointer<QWidget> m_host;
    Status m_status = Error;
};

#endif // TABLERECOGNIZETOAST_H
