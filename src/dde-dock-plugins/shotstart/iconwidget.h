// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONWIDGET_H
#define ICONWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QIcon>
#include <DWidget>
#include <DFontSizeManager>
#include <QBoxLayout>
#include <QLabel>

#include "iconwidget_interface.h"

DWIDGET_USE_NAMESPACE

class IconWidget : public DWidget
{
    Q_OBJECT
public:
    explicit IconWidget(DWidget *parent = nullptr);
    ~IconWidget() override;
    bool enabled();

    const QString itemContextMenu();
    void invokedMenuItem(const QString &menuId);
    QString getSysShortcuts(const QString type);
    QString getDefaultValue(const QString type);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void onPropertyChanged(const QString &property, const QVariant &value);
    void onPositionChanged(int value);

private:
    const QPixmap loadSvg(const QString &fileName, const QSize &size) const;
    void updateIcon();

private:
    iconwidget_interface *m_dockInter;  // DBus 接口
    bool m_hover = false;       // 鼠标是否悬浮
    bool m_pressed = false;     // 鼠标是否按下
    QIcon m_icon;
    QPixmap *m_blgPixmap;
    QBoxLayout *centralLayout;

    QLabel *m_iconLabel;        // 新增图标标签
    int m_position;             // 新增位置属性
/**
* @brief m_systemVersion 获取系统镜像版本
*/
    int m_systemVersion;
};

#endif // ICONWIDGET_H
