#include "colortoolwidget.h"
#include "toolbutton.h"
#include "../utils/configsettings.h"

#include <DSlider>
#include <QLineEdit>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QStyleFactory>
#include <QDebug>
#include <DPalette>

DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 160;
const int TOOLBAR_WIDTH = 40;
const int BUTTON_SPACING = 4;
const int COLOR_NUM = 16;
const QSize TOOL_BUTTON_SIZE = QSize(38, 38);
const QSize SPLITTER_SIZE = QSize(30, 1);
const QSize MIN_TOOL_BUTTON_SIZE = QSize(35, 30);
}

ColorToolWidget::ColorToolWidget(QWidget *parent) : DLabel(parent)
{
    initWidget();
    m_function = "rectangle";
}

ColorToolWidget::~ColorToolWidget()
{

}

void ColorToolWidget::initWidget()
{
    setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
    setMouseTracking(true);
    setAcceptDrops(true);
    initColorLabel();
}

void ColorToolWidget::initColorLabel()
{
    QList<ToolButton *> toolBtnList;
    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(true);
    QPalette pa;

    ToolButton *redBtn = new ToolButton();
    redBtn->setCheckable(true);
//    redBtn->setText(tr("Red"));
    redBtn->setToolTip(tr("Red"));
    redBtn->setObjectName("redBtn");
    redBtn->setFixedSize(TOOL_BUTTON_SIZE);
    redBtn->setIcon(QIcon(":/resources/images/color/color3_checked.png"));
    toolBtnList.append(redBtn);

    ToolButton *yellowBtn = new ToolButton();
//    yellowBtn->setText(tr("yellow"));
    yellowBtn->setToolTip(tr("yellow"));
    yellowBtn->setObjectName("yellowBtn");
    yellowBtn->setFixedSize(TOOL_BUTTON_SIZE);
    yellowBtn->setIcon(QIcon(":/resources/images/color/color1_checked.png"));
    toolBtnList.append(yellowBtn);

    ToolButton *blueBtn = new ToolButton();
//    blueBtn->setText(tr("blue"));
    blueBtn->setToolTip(tr("blue"));
    blueBtn->setObjectName("blueBtn");
    blueBtn->setFixedSize(TOOL_BUTTON_SIZE);
    blueBtn->setIcon(QIcon(":/resources/images/color/color8_checked.png"));
    toolBtnList.append(blueBtn);

    ToolButton *greenBtn = new ToolButton();
//    greenBtn->setText(tr("green"));
    greenBtn->setToolTip(tr("green"));
    greenBtn->setObjectName("greenBtn");
    greenBtn->setFixedSize(TOOL_BUTTON_SIZE);
    greenBtn->setIcon(QIcon(":/resources/images/color/color10_checked.png"));
    toolBtnList.append(greenBtn);

    m_baseLayout = new QVBoxLayout();
    m_baseLayout->setMargin(0);
    m_baseLayout->setSpacing(0);
//    m_baseLayout->addSpacing(4);

//    ToolButton *t_seperator = new ToolButton();
//    pa = t_seperator->palette();
//    QColor t_color = pa.color(DPalette::Text);
//    pa.setColor(DPalette::Light, QColor("#414D68"));
//    pa.setColor(DPalette::Dark, QColor("#414D68"));
//    t_seperator->setDisabled(true);
//    t_seperator->setPalette(pa);
//    t_seperator->setFixedSize(SPLITTER_SIZE);
//    m_baseLayout->addWidget(t_seperator, 0, Qt::AlignHCenter);

    for (int k = 0; k < toolBtnList.length(); k++) {
        m_baseLayout->addWidget(toolBtnList[k]);
        m_baseLayout->addSpacing(BUTTON_SPACING);

        buttonGroup->addButton(toolBtnList[k]);
    }

    setLayout(m_baseLayout);

    connect(buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {

        QPalette pa;
        if (redBtn->isChecked()) {
            pa = redBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, Qt::black);
            pa.setColor(QPalette::Light, Qt::black);
            redBtn->setPalette(pa);

            m_isChecked = true;
            redBtn->update();
            emit colorChecked("red");

            ConfigSettings::instance()->setValue("common", "color_index", 0);
            ConfigSettings::instance()->setValue("rectangle", "color_index", 0);
            ConfigSettings::instance()->setValue("oval", "color_index", 0);
            ConfigSettings::instance()->setValue("arrow", "color_index", 0);
            ConfigSettings::instance()->setValue("line", "color_index", 0);
            ConfigSettings::instance()->setValue("text", "color_index", 0);
        }

        else {
            pa = redBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor("#414d68"));
            pa.setColor(QPalette::Dark, QColor("#e3e3e3"));
            pa.setColor(QPalette::Light, QColor("#e6e6e6"));
            redBtn->setPalette(pa);
        }

        if (yellowBtn->isChecked()) {
            pa = yellowBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, Qt::black);
            pa.setColor(QPalette::Light, Qt::black);
            yellowBtn->setPalette(pa);

            m_isChecked = true;
            yellowBtn->update();
            emit colorChecked("yellow");
            ConfigSettings::instance()->setValue("common", "color_index", 1);
            ConfigSettings::instance()->setValue("rectangle", "color_index", 1);
            ConfigSettings::instance()->setValue("oval", "color_index", 1);
            ConfigSettings::instance()->setValue("arrow", "color_index", 1);
            ConfigSettings::instance()->setValue("line", "color_index", 1);
            ConfigSettings::instance()->setValue("text", "color_index", 1);
        }

        else {
            pa = yellowBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor("#414d68"));
            pa.setColor(QPalette::Dark, QColor("#e3e3e3"));
            pa.setColor(QPalette::Light, QColor("#e6e6e6"));
            yellowBtn->setPalette(pa);
        }

        if (blueBtn->isChecked()) {
            pa = blueBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, Qt::black);
            pa.setColor(QPalette::Light, Qt::black);
            blueBtn->setPalette(pa);

            m_isChecked = true;
            blueBtn->update();
            emit colorChecked("blue");

            ConfigSettings::instance()->setValue("common", "color_index", 2);
            ConfigSettings::instance()->setValue("rectangle", "color_index", 2);
            ConfigSettings::instance()->setValue("oval", "color_index", 2);
            ConfigSettings::instance()->setValue("arrow", "color_index", 2);
            ConfigSettings::instance()->setValue("line", "color_index", 2);
            ConfigSettings::instance()->setValue("text", "color_index", 2);
        }

        else {
            pa = blueBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor("#414d68"));
            pa.setColor(QPalette::Dark, QColor("#e3e3e3"));
            pa.setColor(QPalette::Light, QColor("#e6e6e6"));
            blueBtn->setPalette(pa);
        }

        if (greenBtn->isChecked()) {
            pa = greenBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, Qt::black);
            pa.setColor(QPalette::Light, Qt::black);
            greenBtn->setPalette(pa);

            m_isChecked = true;
            greenBtn->update();
            emit colorChecked("green");

            ConfigSettings::instance()->setValue("common", "color_index", 3);
            ConfigSettings::instance()->setValue("rectangle", "color_index", 3);
            ConfigSettings::instance()->setValue("oval", "color_index", 3);
            ConfigSettings::instance()->setValue("arrow", "color_index", 3);
            ConfigSettings::instance()->setValue("line", "color_index", 3);
            ConfigSettings::instance()->setValue("text", "color_index", 3);
        }

        else {
            pa = greenBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor("#414d68"));
            pa.setColor(QPalette::Dark, QColor("#e3e3e3"));
            pa.setColor(QPalette::Light, QColor("#e6e6e6"));
            greenBtn->setPalette(pa);
        }
    });
    redBtn->click();
    ConfigSettings::instance()->setValue("common", "color_index", 0);
    ConfigSettings::instance()->setValue("rectangle", "color_index", 0);
    ConfigSettings::instance()->setValue("oval", "color_index", 0);
    ConfigSettings::instance()->setValue("arrow", "color_index", 0);
    ConfigSettings::instance()->setValue("line", "color_index", 0);
    ConfigSettings::instance()->setValue("text", "color_index", 0);
}

void ColorToolWidget::setFunction(const QString &func)
{
    m_function = func;
}
