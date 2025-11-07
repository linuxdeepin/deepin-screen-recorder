#include "aiassistantwidget.h"
#include "../utils/log.h"

#include <QHBoxLayout>
#include <QButtonGroup>
#include <DBlurEffectWidget>
#include <DFontSizeManager>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QIcon>

DWIDGET_USE_NAMESPACE

namespace {
    const QSize iconSize(24, 24); 
    const int AIAssistantWidgetHeight = 68;
    const int extraMargin = 20;
}

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
    hLayout->setSpacing(0);
    

    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(true);
    
    
    m_explainButton = new DToolButton(this);
    m_explainButton->setText(tr("Explain"));
    m_explainButton->setIcon(QIcon::fromTheme("explain"));
    m_explainButton->setIconSize(iconSize);
    m_explainButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_explainButton->setCheckable(true);
    m_explainButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    
    m_summarizeButton = new DToolButton(this);
    m_summarizeButton->setText(tr("Summary"));
    m_summarizeButton->setIcon(QIcon::fromTheme("summary"));
    m_summarizeButton->setIconSize(iconSize);
    m_summarizeButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_summarizeButton->setCheckable(true);
    m_summarizeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    
    m_translateButton = new DToolButton(this);
    m_translateButton->setText(tr("Translate"));
    m_translateButton->setIcon(QIcon::fromTheme("translate"));
    m_translateButton->setIconSize(iconSize);
    m_translateButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_translateButton->setCheckable(true);
    m_translateButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    
    m_askAIButton = new DToolButton(this);
    m_askAIButton->setText(tr("Ask AI"));
    m_askAIButton->setIcon(QIcon::fromTheme("askai"));
    m_askAIButton->setIconSize(iconSize);
    m_askAIButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_askAIButton->setCheckable(true);
    m_askAIButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // 绑定字体大小
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
            this, &AIAssistantWidget::onToolButtonClicked);
    
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_blurArea);
    setLayout(mainLayout);
}

QSize AIAssistantWidget::sizeHint() const
{
    QSize blurAreaSize = m_blurArea->sizeHint();
    int totalWidth = blurAreaSize.width() + extraMargin;  
    return QSize(totalWidth, AIAssistantWidgetHeight);
}

void AIAssistantWidget::onToolButtonClicked()
{
    QButtonGroup *buttonGroup = qobject_cast<QButtonGroup *>(sender());
    if (buttonGroup) {
        int functionId = buttonGroup->checkedId();
        AIFunction function = static_cast<AIFunction>(functionId);
        qCDebug(dsrApp) << "AI function selected:" << function;
        emit functionSelected(function);
    }
}
