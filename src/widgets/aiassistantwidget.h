#ifndef AIASSISTANTWIDGET_H
#define AIASSISTANTWIDGET_H

#include <DWidget>
#include <DToolButton>
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
    
    QSize sizeHint() const override;
    
signals:
    void functionSelected(AIFunction function);
    void requestClose();

private slots:
    void onToolButtonClicked();

private:
    DToolButton *m_explainButton;
    DToolButton *m_summarizeButton;
    DToolButton *m_translateButton;
    DToolButton *m_askAIButton;
    
    DBlurEffectWidget *m_blurArea;
};

#endif // AIASSISTANTWIDGET_H
