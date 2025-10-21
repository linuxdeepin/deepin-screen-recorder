#ifndef AIASSISTANTWIDGET_H
#define AIASSISTANTWIDGET_H

#include <DWidget>
#include <DRadioButton>
#include <DBlurEffectWidget>

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
    
signals:
    void functionSelected(AIFunction function);
    void requestClose();

private slots:
    void onRadioButtonClicked();

private:
    DRadioButton *m_explainButton;
    DRadioButton *m_summarizeButton;
    DRadioButton *m_translateButton;
    DRadioButton *m_askAIButton;
    
    DBlurEffectWidget *m_blurArea;
};

#endif // AIASSISTANTWIDGET_H
