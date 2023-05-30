// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIMEWIDGET_H
#define TIMEWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QTime>
#include <QIcon>
#include <DWidget>
#include <com_deepin_dde_daemon_dock.h>
#include <DFontSizeManager>
#include <QBoxLayout>
#include <QLabel>

DWIDGET_USE_NAMESPACE
using DBusDock = com::deepin::dde::daemon::Dock;

class IconWidget : public QWidget
{
    Q_OBJECT
public:
    explicit IconWidget(QWidget *parent = nullptr);
    ~IconWidget() override;
    bool enabled();

    const QString itemContextMenu();
    void invokedMenuItem(const QString &menuId);
    QString getSysShortcuts(const QString type);
    QString getDefaultValue(const QString type);

    QPixmap iconPixMap(QIcon icon, QSize size);
    /**
     * @brief start:开始计时
     */
    void start();

    /**
     * @brief stop:停止计时
     */
    void stop();

    QString getTimeStr();
protected:
    void paintEvent(QPaintEvent *e) override;
protected slots:
    void onTimeout();
private:
    const QPixmap loadSvg(const QString &fileName, const QSize &size) const;


private:
    bool m_hover = false;       // 鼠标是否悬浮
    bool m_pressed = false;     // 鼠标是否按下
    QIcon m_icon;
    QPixmap *m_blgPixmap;
    QBoxLayout *centralLayout;

    QTimer *m_timer;
    QTime m_baseTime;
    QString m_showTimeStr;
};

#endif // TIMEWIDGET_H
