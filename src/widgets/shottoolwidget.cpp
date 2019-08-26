#include "shottoolwidget.h"
#include "toolbutton.h"

#include <DSlider>
#include <QLineEdit>
#include <QMenu>
#include <QAction>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QStyleFactory>
#include <QLine>
#include <DPalette>
#include <QDebug>
#include <DFrame>

DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 190;
const int TOOLBAR_WIDTH = 40;
const int BUTTON_SPACING = 3;
const int SHOT_BUTTON_SPACING = 3;
const int COLOR_NUM = 16;
const QSize TOOL_BUTTON_SIZE = QSize(38, 38);
const QSize SPLITTER_SIZE = QSize(30, 1);
const QSize MIN_TOOL_BUTTON_SIZE = QSize(35, 30);
}

ShotToolWidget::ShotToolWidget(QWidget *parent) : DStackedWidget(parent)
{
    initWidget();
}

ShotToolWidget::~ShotToolWidget()
{

}

void ShotToolWidget::initWidget()
{
    setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);

    initRectLabel();
    initCircLabel();
    initLineLabel();
    initPenLabel();
    initTextLabel();
    setCurrentWidget(m_rectSubTool);
}

void ShotToolWidget::initRectLabel()
{
    m_rectSubTool = new DLabel(this);
    QPalette pa;

    //选择粗细按钮组
    QButtonGroup *t_thicknessBtnGroup = new QButtonGroup();
    t_thicknessBtnGroup->setExclusive(true);

    //选择功能按钮组
    QButtonGroup *t_funcBtnGroup = new QButtonGroup();
    t_funcBtnGroup->setExclusive(true);
    QList<ToolButton *> btnList;

    //粗细程度１级按钮
    ToolButton *thickOneBtn = new ToolButton();
    thickOneBtn->setCheckable(true);
    thickOneBtn->setText(tr("1st"));
    thickOneBtn->setObjectName("thickOneBtn");
    thickOneBtn->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(thickOneBtn);

    //粗细程度２级按钮
    ToolButton *thickTwoBtn = new ToolButton();
    thickTwoBtn->setText(tr("2nd"));
    thickTwoBtn->setObjectName("thickTwoBtn");
    thickTwoBtn->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(thickTwoBtn);

    //粗细程度3级按钮
    ToolButton *thickThreeBtn = new ToolButton();
    thickThreeBtn->setText(tr("3th"));
    thickThreeBtn->setObjectName("thickThreeBtn");
    thickThreeBtn->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(thickThreeBtn);

    QVBoxLayout *rectLayout = new QVBoxLayout();
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(5);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
//        rectLayout->addSpacing(BUTTON_SPACING);

        t_thicknessBtnGroup->addButton(btnList[i]);
    }

//    DSplitter *thickSplitter = new DSplitter();
//    pa = thickSplitter->palette();
//    pa.setColor(DPalette::Base, Qt::black);
//    thickSplitter->setPalette(pa);
//    thickSplitter->setFixedSize(SPLITTER_SIZE);
//    rectLayout->addWidget(thickSplitter);

    rectLayout->addSpacing(BUTTON_SPACING);
    ToolButton *t_seperator = new ToolButton();
    pa = t_seperator->palette();
    pa.setColor(DPalette::Light, Qt::black);
    pa.setColor(DPalette::Dark, Qt::black);
    t_seperator->setDisabled(true);
    t_seperator->setPalette(pa);
    t_seperator->setFixedSize(SPLITTER_SIZE);
    rectLayout->addWidget(t_seperator, 0, Qt::AlignHCenter);
    rectLayout->addSpacing(BUTTON_SPACING);

    btnList.clear();

    //模糊按钮
    ToolButton *blurButton = new ToolButton();
    blurButton->setText(tr("blur"));
    blurButton->setObjectName("blurButton");
    blurButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(blurButton);

    //马赛克按钮
    ToolButton *mosaicBtn = new ToolButton();
    mosaicBtn->setText(tr("mosaic"));
    mosaicBtn->setObjectName("mosaicBtn");
    mosaicBtn->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(mosaicBtn);

    for (int j = 0; j < btnList.length(); j++) {
        rectLayout->addWidget(btnList[j]);
//        rectLayout->addSpacing(BUTTON_SPACING);

        t_funcBtnGroup->addButton(btnList[j]);
    }

//    rectLayout->addSpacing(BUTTON_SPACING);

//    rectLayout->addSpacing(BUTTON_SPACING);

    m_rectSubTool->setLayout(rectLayout);
    addWidget(m_rectSubTool);
}

void ShotToolWidget::initCircLabel()
{
    return;
}

void ShotToolWidget::initLineLabel()
{
    return;
}

void ShotToolWidget::initPenLabel()
{
    return;
}

void ShotToolWidget::initTextLabel()
{
    return;
}

void ShotToolWidget::switchContent(QString shapeType)
{
    return;
}
