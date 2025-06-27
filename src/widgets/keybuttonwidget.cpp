// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "keybuttonwidget.h"
#include "../utils.h"
#include "../utils/configsettings.h"
#include "../utils/log.h"
#include <DWindowManagerHelper>

DWIDGET_USE_NAMESPACE

namespace {
const int _BUTTON_HEIGHT = 35;
const int _BUTTON_WIDTH = 65;

//const int LONG_BUTTON_HEIGHT = 40;
//const int LONG_BUTTON_WIDTH = 80;
//const int BTN_RADIUS = 3;
}


KeyButtonWidget::KeyButtonWidget(DWidget *parent) : DBlurEffectWidget(parent)
{
    qCDebug(dsrApp) << "KeyButtonWidget constructor entered";
    if(DWindowManagerHelper::instance()->hasComposite()){
        qCDebug(dsrApp) << "Composite manager detected, setting blur radius to 15";
        setBlurRectXRadius(15);
        setBlurRectYRadius(15);
    }else {
        qCDebug(dsrApp) << "No composite manager, setting blur radius to 0";
        setBlurRectXRadius(0);
        setBlurRectYRadius(0);
    }
    setRadius(30);
    setMode(DBlurEffectWidget::GaussianBlur);
    setBlurEnabled(true);
//    setBlendMode(DBlurEffectWidget::InWindowBlend);
//    setMaskColor(QColor(255, 255, 255, 140));

    if (Utils::themeType == 1) {
        qCDebug(dsrApp) << "Theme type is 1, setting mask color to white (140 alpha)";
        setMaskColor(QColor(255, 255, 255, 140));
    } else {
        qCDebug(dsrApp) << "Theme type is not 1, setting mask color to black (76 alpha)";
        setMaskColor(QColor(0, 0, 0, 76));
    }
    //设置透明效果
    setFixedWidth(_BUTTON_WIDTH);
    setFixedHeight(_BUTTON_HEIGHT);
    m_word = new DLabel();
    DFontSizeManager::instance()->bind(m_word, DFontSizeManager::T7);
    DPalette pa;
//    pa.setColor(DPalette::Text, Qt::black);
    this->setPalette(pa);
//    QFont t_wordFont;
//    t_wordFont.setPixelSize(13);
//    m_word->setFont(t_wordFont);
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(2);
    hLayout->addWidget(m_word, 0, Qt::AlignVCenter | Qt::AlignHCenter);
    this->setLayout(hLayout);
    if (!Utils::isWaylandMode) {
        qCDebug(dsrApp) << "Setting up X11 input event passthrough";
        Utils::passInputEvent(static_cast<int>(this->winId()));
    }
}

KeyButtonWidget::~KeyButtonWidget()
{
    qCDebug(dsrApp) << "KeyButtonWidget destructor entered";
}

void KeyButtonWidget::setKeyLabelWord(const QString &keyWord)
{
    qCDebug(dsrApp) << "setKeyLabelWord called with:" << keyWord;
    m_keyword = keyWord;
    m_word->setText(m_keyword);
    this->repaint();
}
