/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     Zheng Youge<youge.zheng@deepin.com>
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

#ifndef SUBTOOLWIDGET_H
#define SUBTOOLWIDGET_H

#include <QObject>
#include <DLabel>
#include <DStackedWidget>
#include "toolbutton.h"
#include "../utils/saveutils.h"
#include "filter.h"
DWIDGET_USE_NAMESPACE

class SubToolWidget : public DStackedWidget
{
    Q_OBJECT
public:
    explicit SubToolWidget(DWidget *parent = nullptr);
    ~SubToolWidget();

    void initWidget();
    void initRecordLabel();
    void initShotLabel();
//    void initVirtualCard();
    void installTipHint(QWidget *w, const QString &hintstr);
    void installHint(QWidget *w, QWidget *hint);
signals:
    void keyBoardButtonClicked(bool checked);
    void mouseBoardButtonClicked(bool checked);
    void microphoneActionChecked(bool checked);
    void systemAudioActionChecked(bool checked);
    void gifActionChecked(bool checked);
    void mp4ActionChecked(bool checked);
    void cameraActionChecked(bool checked);
    void videoFrameRateChanged(int frameRate);
    void changeShotToolFunc(const QString &func);
    void saveMethodChanged(SaveAction saveact, const QString path);
    void saveToClipBoard(bool isClip);
public slots:
    void switchContent(QString shapeType);
    void systemAudioActionCheckedSlot(bool checked);
    void changeArrowAndLineFromSideBar(int line);
    void setRecordLaunchMode(bool recordLaunch);
    void setVideoButtonInitFromSub();
    void shapeClickedFromWidget(QString shape);
    void setMicroPhoneEnable(bool status);
    void setCameraDeviceEnable(bool status);
    void setSystemAudioEnable(bool status);
    void setIsZhaoxinPlatform(bool isZhaoxin);
private:
    DLabel *m_recordSubTool;
    DLabel *m_shotSubTool;
    QString m_currentType;
    QAction *m_systemAudioAction;
    ToolButton *m_rectButton;
    ToolButton *m_circleButton;
    ToolButton *m_lineButton;
    ToolButton *m_penButton;
    ToolButton *m_textButton;
    ToolButton *m_cameraButton;
    ToolButton *m_keyBoardButton;
    ToolButton *m_mouseButton;
    ToolButton *m_optionButton;
    ToolButton *m_shotOptionButton;
    QAction *m_microphoneAction;
    bool m_haveMicroPhone = false;
    bool m_haveSystemAudio = false;
    int m_lineflag;
    int m_themeType;
    bool m_isZhaoxinInSub = false;
    HintFilter *hintFilter = nullptr;
};

#endif // SUBTOOLWIDGET_H
