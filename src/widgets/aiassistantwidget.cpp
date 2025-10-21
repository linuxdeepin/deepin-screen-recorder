#include "aiassistantwidget.h"
#include "../utils/log.h"

#include <QHBoxLayout>
#include <QButtonGroup>
#include <DBlurEffectWidget>
#include <DFontSizeManager>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusMessage>

DWIDGET_USE_NAMESPACE

AIAssistantWidget::AIAssistantWidget(QWidget *parent) : DWidget(parent)
{
    m_blurArea = new DBlurEffectWidget(this);
    m_blurArea->setBlurRectXRadius(7);
    m_blurArea->setBlurRectYRadius(7);
    m_blurArea->setRadius(15);
    m_blurArea->setMode(DBlurEffectWidget::GaussianBlur);
    m_blurArea->setBlurEnabled(true);
    m_blurArea->setBlendMode(DBlurEffectWidget::InWidgetBlend);
    m_blurArea->setMaskColor(QColor(255, 255, 255, 0));
    
    QHBoxLayout *hLayout = new QHBoxLayout(m_blurArea);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(6); 
    

    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(true);
    
    m_explainButton = new DRadioButton(tr("Explain"));
    m_summarizeButton = new DRadioButton(tr("Summary"));
    m_translateButton = new DRadioButton(tr("Translate"));
    m_askAIButton = new DRadioButton(tr("Ask AI"));

    DFontSizeManager::instance()->bind(m_explainButton, DFontSizeManager::T6);
    DFontSizeManager::instance()->bind(m_summarizeButton, DFontSizeManager::T6);
    DFontSizeManager::instance()->bind(m_translateButton, DFontSizeManager::T6);
    DFontSizeManager::instance()->bind(m_askAIButton, DFontSizeManager::T6);
    
    buttonGroup->addButton(m_explainButton, Explain);
    buttonGroup->addButton(m_summarizeButton, Summarize);
    buttonGroup->addButton(m_translateButton, Translate);
    buttonGroup->addButton(m_askAIButton, AskAI);
    
    hLayout->addWidget(m_explainButton);
    hLayout->addWidget(m_summarizeButton);
    hLayout->addWidget(m_translateButton);
    hLayout->addWidget(m_askAIButton);
    
    connect(buttonGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), 
            this, &AIAssistantWidget::onRadioButtonClicked);
    
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_blurArea);
    setLayout(mainLayout);
}

void AIAssistantWidget::onRadioButtonClicked()
{
    QButtonGroup *buttonGroup = qobject_cast<QButtonGroup *>(sender());
    if (buttonGroup) {
        int functionId = buttonGroup->checkedId();
        AIFunction function = static_cast<AIFunction>(functionId);
        qCDebug(dsrApp) << "AI function selected:" << function;
        emit functionSelected(function);
    }
}
