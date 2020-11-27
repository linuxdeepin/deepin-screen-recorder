/**
 * Copyright (C) 2020 UOS Technology Co., Ltd.
 *
 * to mark the desktop UI
 **/

#ifndef SHOT_RECORDER_ACCESSIBLE_UI_DEFINE_H
#define SHOT_RECORDER_ACCESSIBLE_UI_DEFINE_H

#include <QString>
#include <QObject>

// 使用宏定义，方便国际化操作



// 开始录屏，截图按钮
#define AC_MAINWINDOW_MAINSHOTBTN QObject::tr("main_shot_but") // 截图按钮
#define AC_MAINWINDOW_MAINRECORDBTN QObject::tr("main_record_but") // 录屏按钮

// 截图录屏切换按钮
#define AC_MAINTOOLWIDGET_RECORD_BUT QObject::tr("to_record_but") //切换到录屏功能按钮
#define AC_MAINTOOLWIDGET_SHOT_BUT QObject::tr("to_shot_but")// 切换到截图功能按钮

// 关闭按钮
#define AC_TOOLBARWIDGET_CLOSE_BUTTON QObject::tr("close_button")// 关闭按钮


// 底部工具栏按钮
#define AC_SUBTOOLWIDGET_AUDIO_BUTTON QObject::tr("audio_button")// 录屏 音频按钮
#define AC_SUBTOOLWIDGET_KEYBOARD_BUTTON QObject::tr("keyboard_button")// 录屏 显示按键按钮
#define AC_SUBTOOLWIDGET_CAMERA_BUTTON QObject::tr("camera_button")//录屏 摄像头按钮
#define AC_SUBTOOLWIDGET_MOUSE_BUTTON QObject::tr("mouse_button")//录屏 显示鼠标按钮
#define AC_SUBTOOLWIDGET_RECORD_OPTION_BUT QObject::tr("record_option_but") //录屏选项按钮


#define AC_SUBTOOLWIDGET_RECT_BUTTON QObject::tr("rect_button")// 截图 矩形工具按钮
#define AC_SUBTOOLWIDGET_CIRCL_BUTTON QObject::tr("circl_button")//截图 椭圆工具按钮
#define AC_SUBTOOLWIDGET_LINE_BUTTON QObject::tr("line_button")// 截图 直线按钮
#define AC_SUBTOOLWIDGET_PEN_BUTTON QObject::tr("pen_button")// 截图 画笔按钮
#define AC_SUBTOOLWIDGET_TEXT_BUTTON QObject::tr("text_button")// 截图文本按钮
#define AC_SUBTOOLWIDGET_SHOT_OPTION_BUT QObject::tr("shot_option_but") // 截图 选项按钮


// 边侧工具栏按钮
#define AC_SHOTTOOLWIDGET_THICK_ONE_RECT QObject::tr("thick_one_rect") // 矩形宽度1
#define AC_SHOTTOOLWIDGET_THICK_TWO_RECT QObject::tr("thick_two_rect") // 矩形宽度2
#define AC_SHOTTOOLWIDGET_THICK_THREE_RECT QObject::tr("thick_three_rect") //矩形宽度3
#define AC_SHOTTOOLWIDGET_BLUR_RECT QObject::tr("thick_blur_rect")//矩形水滴
#define AC_SHOTTOOLWIDGET_MOSAIC_RECT QObject::tr("thick_mosaic_rect")//矩形马赛克

#define AC_SHOTTOOLWIDGET_THICK_ONE_CIRC QObject::tr("thick_one_circ")// 椭圆宽度1
#define AC_SHOTTOOLWIDGET_THICK_TWO_CIRC QObject::tr("thick_two_circ")// 椭圆宽度2
#define AC_SHOTTOOLWIDGET_THICK_THREE_CIRC QObject::tr("thick_three_circ")// 椭圆宽度3
#define AC_SHOTTOOLWIDGET_BLUR_CIRC QObject::tr("thick_blur_circ")//椭圆水滴
#define AC_SHOTTOOLWIDGET_MOSAIC_CIRC QObject::tr("thick_mosaic_circ")//椭圆马赛克

#define AC_SHOTTOOLWIDGET_THICK_ONE_LINE QObject::tr("thick_one_line")// 直线宽度1
#define AC_SHOTTOOLWIDGET_THICK_TWO_LINE QObject::tr("thick_two_line")// 直线宽度2
#define AC_SHOTTOOLWIDGET_THICK_THREE_LINE QObject::tr("thick_three_line")// 直线宽度3
#define AC_SHOTTOOLWIDGET_LINE_BUTTON QObject::tr("thick_line_line") // 直线按钮
#define AC_SHOTTOOLWIDGET_ARROW_BUTTON QObject::tr("thick_arrow_line")// 箭头按钮

#define AC_SHOTTOOLWIDGET_THICK_ONE_PEN QObject::tr("thick_one_pen")// 画笔宽度1
#define AC_SHOTTOOLWIDGET_THICK_TWO_PEN QObject::tr("thick_two_pen")// 画笔宽度2
#define AC_SHOTTOOLWIDGET_THICK_THREE_PEN QObject::tr("thick_three_pen")// 画笔宽度3

// 颜色按钮
#define AC_COLORTOOLWIDGET_RED_BUT QObject::tr("red_button") // 红色按钮
#define AC_COLORTOOLWIDGET_YELLOW_BUT QObject::tr("yello_button")// 黄色按钮
#define AC_COLORTOOLWIDGET_BLUE_BUT QObject::tr("blue_button")// 蓝色按钮
#define AC_COLORTOOLWIDGET_GREEN_BUT QObject::tr("green_button")//绿色按钮

// 字体大小
#define AC_SHOTTOOLWIDGET_TEXT_FONT_SIZE QObject::tr("text_font_size")// 字体大小

#endif // SHOT_RECORDER_ACCESSIBLE_UI_DEFINE_H
