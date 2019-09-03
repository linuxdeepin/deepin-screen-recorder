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
DWIDGET_USE_NAMESPACE

class SubToolWidget : public DStackedWidget
{
    Q_OBJECT
public:
    explicit SubToolWidget(QWidget *parent = nullptr);
    ~SubToolWidget();

    void initWidget();
    void initRecordLabel();
    void initShotLabel();
    void initVirtualCard();
signals:
    void keyBoardButtonClicked(bool checked);
    void mouseBoardButtonClicked(bool checked);
    void microphoneActionChecked(bool checked);
    void systemAudioActionChecked(bool checked);
    void gifActionChecked(bool checked);
    void mp4ActionChecked(bool checked);
    void videoFrameRateChanged(int frameRate);
    void changeShotToolFunc(const QString &func);
public slots:
    void switchContent(QString shapeType);
    void systemAudioActionCheckedSlot(bool checked);
    void changeArrowAndLineFromSideBar(int line);
private:
    QLabel *m_recordSubTool;
    QLabel *m_shotSubTool;
    QString m_currentType;
    QAction *m_systemAudioAction;
    ToolButton *m_lineButton;
    int m_lineflag;
};

#endif // SUBTOOLWIDGET_H
