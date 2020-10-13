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

#ifndef SUBTOOLBAR_H
#define SUBTOOLBAR_H

#include <DStackedWidget>
#include <DLabel>
#include <DWidget>

DWIDGET_USE_NAMESPACE

enum SaveAction : unsigned int;
class SubToolBar : public DStackedWidget
{
    Q_OBJECT
public:
    explicit SubToolBar(DWidget *parent = 0);
    ~SubToolBar();

    void initWidget();
    void initRectLabel();
    void initArrowLabel();
    void initLineLabel();
    void initTextLabel();
    void initColorLabel();
    void initSaveLabel();

public slots:
    void switchContent(QString shapeType);
    void setSaveOption(SaveAction action);
    void setSaveQualityIndex(int saveQuality);
    //int  getSaveQualityIndex();
    void updateColor(QColor color);

signals:
    void currentColorChanged(QColor color);
    void shapeChanged();
    void saveAction();
    void showSaveTip(QString tips);
    void hideSaveTip();
    void saveBtnPressed(SaveAction index);
    void defaultColorIndexChanged(int index);

private:
    int m_lineWidth;
    int m_saveQuality;
    QString m_currentType;

    DLabel *m_rectLabel;
    DLabel *m_arrowLabel;
    DLabel *m_lineLabel;
    DLabel *m_textLabel;
    DLabel *m_colorLabel;
    DLabel *m_saveLabel;
};
#endif // SUBTOOLBAR_H
