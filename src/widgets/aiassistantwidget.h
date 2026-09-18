// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AIASSISTANTWIDGET_H
#define AIASSISTANTWIDGET_H

#include <DWidget>
#include <DBlurEffectWidget>
#include "toolbutton.h"

DWIDGET_USE_NAMESPACE

class AIAssistantWidget : public DWidget
{
    Q_OBJECT
public:
    enum AIFunction {
        Explain,
        Summarize,
        Translate,
        AskAI
    };
    Q_ENUM(AIFunction)

    explicit AIAssistantWidget(QWidget *parent = nullptr);
    
    QSize sizeHint() const override;
    
signals:
    void functionSelected(AIFunction function);
    void requestClose();

private slots:
    void onToolButtonClicked(int functionId);

private:
    ToolButton *m_explainButton;
    ToolButton *m_summarizeButton;
    ToolButton *m_translateButton;
    ToolButton *m_askAIButton;
    
    DBlurEffectWidget *m_blurArea;
};

#endif // AIASSISTANTWIDGET_H
