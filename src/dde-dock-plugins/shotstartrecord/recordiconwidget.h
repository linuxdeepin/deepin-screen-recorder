// SPDX-FileCopyrightText: 2021-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIMEWIDGET_H
#define TIMEWIDGET_H

#include "recordiconwidget_interface.h"
#include <DWidget>
#include <QSettings>
#include <QTime>
#include <QIcon>
#include <DFontSizeManager>
#include <QBoxLayout>
#include <QLabel>

DWIDGET_USE_NAMESPACE

class RecordIconWidget : public DWidget
{
    Q_OBJECT
public:
    explicit RecordIconWidget(DWidget *parent = nullptr);
    ~RecordIconWidget() override;
    bool enabled();

    const QString itemContextMenu();
    void invokedMenuItem(const QString &menuId);
    QString getSysShortcuts(const QString &type);
    QString getDefaultValue(const QString &type);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void onPositionChanged(int value);
    void onPropertyChanged(const QString &property, const QVariant &value);

private:
    const QPixmap loadSvg(const QString &fileName, const QSize &size) const;
    void updateIcon();

private:
    recordiconwidget_interface *m_dockInter;  // DBus 接口
    bool m_hover = false;       // 鼠标是否悬浮
    bool m_pressed = false;     // 鼠标是否按下
    QIcon m_icon;
    QPixmap *m_blgPixmap;
    QBoxLayout *centralLayout;
    QLabel *m_iconLabel;        // 新增图标标签
    int m_position;             // 新增位置属性
    QPixmap m_pixmap;
};

#endif // TIMEWIDGET_H
