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
#include "keybuttonwidget.h"

DWIDGET_USE_NAMESPACE

namespace {
    const int _BUTTON_HEIGHT = 40;
    const int _BUTTON_WIDTH = 60;

    const int LONG_BUTTON_HEIGHT = 40;
    const int LONG_BUTTON_WIDTH = 80;
    const int BTN_RADIUS = 3;
}


KeyButtonWidget::KeyButtonWidget(QWidget *parent) : DBlurEffectWidget(parent)
{
    setBlurRectXRadius(10);
    setBlurRectYRadius(10);
    setRadius(30);
    setMode(DBlurEffectWidget::GaussianBlur);
    setBlurEnabled(true);
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    setMaskColor(QColor(255, 255, 255, 200));
    //设置透明效果
    setFixedWidth(_BUTTON_WIDTH);
    setFixedHeight(_BUTTON_HEIGHT);
    m_word = new DLabel(this);
    DFontSizeManager::instance()->bind(m_word, DFontSizeManager::T5);
//    QFont t_wordFont;
//    t_wordFont.setPixelSize(18);
//    m_word->setFont(t_wordFont);
    QHBoxLayout* hLayout = new QHBoxLayout(this);
    hLayout->setMargin(0);
    hLayout->setSpacing(2);
    hLayout->addWidget(m_word, 0, Qt::AlignVCenter | Qt::AlignHCenter);
    this->setLayout(hLayout);
}

KeyButtonWidget::~KeyButtonWidget()
{

}

void KeyButtonWidget::setKeyLabelWord(const QString &keyWord)
{
    m_keyword = keyWord;
    m_word->setText(m_keyword);
    this->repaint();
}
