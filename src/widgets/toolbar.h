/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Maintainer: Peng Hui<penghui@deepin.com>
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

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QLabel>
#include <QPainter>
#include <DBlurEffectWidget>
#include <QEvent>
#include <QDebug>

#include "majtoolbar.h"
#include "subtoolbar.h"
#include "maintoolwidget.h"
#include "subtoolwidget.h"

DWIDGET_USE_NAMESPACE

enum SaveAction : unsigned int;
class ToolBarWidget : public DBlurEffectWidget
{
    Q_OBJECT
public:
    ToolBarWidget(QWidget *parent = 0);
    ~ToolBarWidget();

signals:
    void buttonChecked(QString shapeType);
    void expandChanged(bool expand,  QString shapeType);
    void colorChanged(QColor color);
    void saveImage();
    void shapePressed(QString tool);
    void saveBtnPressed(SaveAction index);
    void saveSpecifiedPath();
    void closed();
    void changeFunctionSignal(QString shapeType);
    void keyBoardCheckedSignal(bool checked);
    void microphoneActionCheckedSignal(bool checked);
    void systemAudioActionCheckedSignal(bool checked);
    void cameraActionCheckedSignal(bool checked);
    void mouseCheckedSignalToToolBar(bool checked);
    void gifActionCheckedSignal(bool checked);
    void mp4ActionCheckedSignal(bool checked);
    void frameRateChangedSignal(int frameRate);
    void shotToolChangedSignal(const QString &func);
public slots:
    bool isButtonChecked();
    void setExpand(bool expand, QString shapeType);
    void specifiedSavePath();
    void keyBoardCheckedSlot(bool checked);
    void changeArrowAndLineFromBar(int line);


protected:
    void paintEvent(QPaintEvent *e);
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private:
    MajToolBar *m_majToolbar;
    QLabel *m_hSeparatorLine;
    SubToolBar *m_subToolbar;

    MainToolWidget *m_mainTool;
    SubToolWidget *m_subTool;

    bool  m_expanded;
};

class ToolBar : public QLabel
{
    Q_OBJECT
public:
    ToolBar(QWidget *parent = 0);
    ~ToolBar();

signals:
    void heightChanged();
    void buttonChecked(QString shape);
    void updateColor(QColor color);
    void requestSaveScreenshot();
    void shapePressed(QString tool);
    void saveBtnPressed(SaveAction index);
    void saveSpecifiedPath();
    void closed();
    void currentFunctionToMain(QString shapeType);
    void keyBoardCheckedToMain(bool checked);
    void microphoneActionCheckedToMain(bool checked);
    void systemAudioActionCheckedToMain(bool checked);
    void cameraActionCheckedToMain(bool checked);
    void mouseCheckedToMain(bool checked);
    void gifActionCheckedToMain(bool checked);
    void mp4ActionCheckedToMain(bool checked);
    void frameRateChangedToMain(int frameRate);
    void shotToolChangedToMain(const QString &func);
public slots:
    bool isButtonChecked();
    void setExpand(bool expand, QString shapeType);
    void showAt(QPoint pos);
    void specificedSavePath();
    void currentFunctionMode(QString shapeType);
    void keyBoardCheckedToMainSlot(bool checked);
    void microphoneActionCheckedToMainSlot(bool checked);
    void systemAudioActionCheckedToMainSlot(bool checked);
    void changeArrowAndLineFromMain(int line);
protected:
    void paintEvent(QPaintEvent *e);
    void enterEvent(QEvent *e);
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    ToolBarWidget *m_toolbarWidget;

    bool m_expanded;
};
#endif // TOOLBAR_H
