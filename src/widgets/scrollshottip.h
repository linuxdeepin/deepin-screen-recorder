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
#include <DCommandLinkButton>
#include <DLabel>
#include <DIconButton>

#include <QtDBus/QtDBus>
#include <QHBoxLayout>
#include <QTimer>

DWIDGET_USE_NAMESPACE

enum TipType{
    StartScrollShotTip,  //开始滚动截图前的提示
    ErrorScrollShotTip,  //滚动截图出现错误的提示
    EndScrollShotTip,     //滚动截图到底部出现的提示
    QuickScrollShotTip,    //滚动速度过快出现的提示
    MaxLengthScrollShotTip  //滚动截图拼接已到达最大长度
};

/**
 * @brief 滚动截图的提示
 */
class ScrollShotTip : public DWidget
{

public:
    static const int NUMBER_PADDING_Y;
    static const int TIP_HEIGHT;

    explicit ScrollShotTip(DWidget *parent = 0);
    ~ScrollShotTip();

    Q_OBJECT

public:

    /**
     * @brief 根据提示的类型选取相应的提示方法
     * @param tipText
     */
    void showTip(TipType tipType);

    /**
     * @brief 获取当前的提示类型
     * @return
     */
    TipType getTipType();

    /**
     * @brief 设置滚动截图当前屏幕的背景
     * @param backgroundPixmap
     */
    void setBackgroundPixmap(QPixmap &backgroundPixmap);

signals:
    /**
     * @brief 打开截图录屏帮助文档并定位到滚动截图
     */
    void openScrollShotHelp();

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

    /**
     * @brief 图像拼接长度限制
     */
    void showMaxScrollShotTip();

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
    //警告图片
    DIconButton *m_warmingIconButton;
    //系统主题
    int m_themeType = 0;
    //提示的内容
    QString m_tipText;

    DLabel *m_tipTextLable;
    /**
     * @brief 背景图片
     */
    QPixmap m_backgroundPixmap;


    /**
     * @brief 提示的类型
     * 参见 TipType 定义
     */
    TipType m_tipType;

    /**
     * @brief 帮助文字按钮
     */
    DCommandLinkButton *m_scrollShotHelp;


};

#endif // SCROLLSHOTTIP_H
