/*

* Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.

*

* Author:     wangcong <wangcong@uniontech.com>

*

* Maintainer: wangcong <wangcong@uniontech.com>

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
#ifndef SCROLLSHOTTIP_H
#define SCROLLSHOTTIP_H

#include <DWidget>
#include <QTimer>
#include <QHBoxLayout>
#include <DCommandLinkButton>

DWIDGET_USE_NAMESPACE

enum TipType{
    StartScrollShotTip,  //开始滚动截图前的提示
    ErrorScrollShotTip,  //滚动截图出现错误的提示
    EndScrollShotTip,     //滚动截图到底部出现的提示
    QuickScrollShotTip    //滚动速度过快出现的提示
};

/**
 * @brief 滚动截图的提示
 */
class ScrollShotTip : public DWidget
{

public:
    static const int NUMBER_PADDING_Y;

    explicit ScrollShotTip(DWidget *parent = 0);

    Q_OBJECT

public:

    /**
     * @brief 根据提示的类型选取相应的
     * @param tipText
     */
    void showTip(TipType tipType);

protected:
    /**
     * @brief 开始滚动截图前的提示
     */
    void showStartScrollShotTip();

    /**
     * @brief 滚动截图出现错误的提示
     */
    void showErrorScrollShotTip();

    /**
     * @brief 滚动截图到底部出现的提示
     */
    void showEndScrollShotTip();

    void paintEvent(QPaintEvent *event);

    /**
     * @brief getTooltipBackground 先获取模糊背景图
     * @return
     */
    QPixmap getTooltipBackground();
    /**
     * @brief paintRect  画模糊背景图
     * @param painter
     * @param blurPixmap  矩形框位置的模糊图形
     */
    void paintRect(QPainter &painter,QPixmap &blurPixmap,int radius);

    /**
     * @brief 画提示的背景
     * @param painter
     * @param rect
     * @param textColor
     * @param opacity
     */
    void drawTooltipBackground(QPainter &painter, QRect rect, QString textColor, qreal opacity,int radius);

private:
    QPixmap m_warmingImg;

    //计时器时间
    int m_showCounter;

    //计时器
    QTimer *m_showTimer;
    //系统主题
    int m_themeType = 0;
    //提示的信息
    QString m_tipText;

    TipType m_tipType;



};

#endif // SCROLLSHOTTIP_H
