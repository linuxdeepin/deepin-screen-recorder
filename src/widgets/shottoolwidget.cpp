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
const int TOOLBAR_HEIGHT = 200;
const int TOOLBAR_WIDTH = 40;
const int BUTTON_SPACING = 3;
const int SHOT_BUTTON_SPACING = 3;
const int COLOR_NUM = 16;
const QSize TOOL_BUTTON_SIZE = QSize(38, 38);
const QSize TOOL_SLIDER_SIZE = QSize(38, 180);
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
        t_thicknessBtnGroup->addButton(btnList[i]);
    }

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
        t_funcBtnGroup->addButton(btnList[j]);
    }

//    rectLayout->addSpacing(BUTTON_SPACING);

//    ToolButton *t_seperator1 = new ToolButton();
//    pa = t_seperator1->palette();
//    pa.setColor(DPalette::Light, QColor("#414D68"));
//    pa.setColor(DPalette::Dark, QColor("#414D68"));
//    t_seperator1->setDisabled(true);
//    t_seperator1->setPalette(pa);
//    t_seperator1->setFixedSize(SPLITTER_SIZE);
//    rectLayout->addWidget(t_seperator1, 1, Qt::AlignHCenter);

    m_rectSubTool->setLayout(rectLayout);
    addWidget(m_rectSubTool);
}

void ShotToolWidget::initCircLabel()
{
    m_circSubTool = new DLabel(this);
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
        t_thicknessBtnGroup->addButton(btnList[i]);
    }

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
        t_funcBtnGroup->addButton(btnList[j]);
    }

    m_circSubTool->setLayout(rectLayout);
    addWidget(m_circSubTool);
}

void ShotToolWidget::initLineLabel()
{
    m_lineSubTool = new DLabel(this);
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
        t_thicknessBtnGroup->addButton(btnList[i]);
    }

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

    //直线按钮
    ToolButton *blurButton = new ToolButton();
    blurButton->setText(tr("line"));
    blurButton->setObjectName("blurButton");
    blurButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(blurButton);

    //箭头按钮
    ToolButton *mosaicBtn = new ToolButton();
    mosaicBtn->setText(tr("arrow"));
    mosaicBtn->setObjectName("mosaicBtn");
    mosaicBtn->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(mosaicBtn);

    for (int j = 0; j < btnList.length(); j++) {
        rectLayout->addWidget(btnList[j]);
        t_funcBtnGroup->addButton(btnList[j]);
    }

    m_lineSubTool->setLayout(rectLayout);
    addWidget(m_lineSubTool);
}

void ShotToolWidget::initPenLabel()
{
    m_penSubTool = new DLabel(this);
    QPalette pa;

    //选择粗细按钮组
    QButtonGroup *t_thicknessBtnGroup = new QButtonGroup();
    t_thicknessBtnGroup->setExclusive(true);

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
        t_thicknessBtnGroup->addButton(btnList[i]);
    }

    rectLayout->addSpacing(70);
    ToolButton *t_seperator = new ToolButton();
    pa = t_seperator->palette();
    pa.setColor(DPalette::Light, Qt::black);
    pa.setColor(DPalette::Dark, Qt::black);
    t_seperator->setDisabled(true);
    t_seperator->setPalette(pa);
    t_seperator->setFixedSize(SPLITTER_SIZE);
    rectLayout->addWidget(t_seperator, 0, Qt::AlignHCenter);
    rectLayout->addSpacing(70);

    btnList.clear();

    m_penSubTool->setLayout(rectLayout);
    addWidget(m_penSubTool);
}

void ShotToolWidget::initTextLabel()
{
    m_textSubTool = new DLabel(this);

    DSlider *t_textFontSize = new DSlider();
    t_textFontSize->setOrientation(Qt::Vertical);
    t_textFontSize->setFixedSize(TOOL_SLIDER_SIZE);

    QVBoxLayout *rectLayout = new QVBoxLayout();
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(5);

    rectLayout->addWidget(t_textFontSize);

    m_textSubTool->setLayout(rectLayout);
    addWidget(m_textSubTool);
}

void ShotToolWidget::switchContent(QString shapeType)
{
    if (!shapeType.isEmpty()) {
        if (shapeType == "rect") {
            setCurrentWidget(m_rectSubTool);
        }

        if (shapeType == "circ") {
            setCurrentWidget(m_circSubTool);
        }

        if (shapeType == "line") {
            setCurrentWidget(m_lineSubTool);
        }

        if (shapeType == "pen") {
            setCurrentWidget(m_penSubTool);
        }

        if (shapeType == "text") {
            setCurrentWidget(m_textSubTool);
        }
    }
}
