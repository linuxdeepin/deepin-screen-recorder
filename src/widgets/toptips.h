// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOPTIPS_H
#define TOPTIPS_H

#include <DLabel>
#include <DWidget>
#include <QLabel>

DWIDGET_USE_NAMESPACE
/**
 * @brief The TopTips class 捕捉区域大小提示界面（除滚动截图外）
 */
class TopTips : public QLabel
{
    Q_OBJECT
public:
    explicit TopTips(DWidget *parent = nullptr);
    ~TopTips() override;
    void setRecorderTipsInfo(const bool showState);
    void setFullScreenRecord(const bool flag);
public slots:
    void setContent(const QSize &rect);
    void updateTips(QPoint pos, const QSize &rect);

protected:
    //void mousePressEvent(QMouseEvent *ev) override;
    bool m_showRecorderTips = false;
    int m_width = 0;
    int m_height = 0;

    /**
     * @brief m_isFullScreenRecord 快捷全屏录制的标志
     * 快捷全屏录制时，不需要显示此界面
     */
    bool m_isFullScreenRecord = false;
};
#endif // TOPTIPS_H
