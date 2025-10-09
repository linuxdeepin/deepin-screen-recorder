// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "subtoolwidget.h"
#include "accessibility/acTextDefine.h"
#include "settings.h"
#include "../../utils/log.h"
#include "pinsavemenumanager.h"

#include <QActionGroup>
#include <QFileInfo>
#include <QHelpEvent>
#include <QToolTip>
#include <DFontSizeManager>
#include <QDir>

#define THEMETYPE 1 // 主题颜色为浅色

SubToolWidget::SubToolWidget(DWidget *parent): DStackedWidget(parent)
{
    qCDebug(dsrApp) << "SubToolWidget constructor called.";
    initShotLable();
    qCDebug(dsrApp) << "initShotLable finished.";
    setCurrentWidget(m_shotSubTool);
    qCDebug(dsrApp) << "Current widget set to m_shotSubTool.";
    
    // 初始化完成后，更新保存按钮的提示文本
    updateSaveButtonTip();
}

// 初始化贴图工具栏按钮
void SubToolWidget::initShotLable()
{
    qCDebug(dsrApp) << "Initializing pin screenshot toolbar buttons.";
    m_shotSubTool = new DLabel(this);
    qCDebug(dsrApp) << "m_shotSubTool DLabel created.";
    // ocr按钮
    m_ocrButton = new ToolButton(this);
    m_ocrButton->setObjectName(AC_SUBTOOLWIDGET_PIN_OCR_BUT);
    m_ocrButton->setAccessibleName(AC_SUBTOOLWIDGET_PIN_OCR_BUT);
    m_ocrButton->setIconSize(QSize(32, 32));
    m_ocrButton->setFixedSize(32, 32);
    m_ocrButton->setIcon(QIcon::fromTheme("ocr-normal"));
    m_ocrButton->setToolTip(tr("Extract Text"));
    connect(m_ocrButton, SIGNAL(clicked()), this, SIGNAL(signalOcrButtonClicked()));
    qCDebug(dsrApp) << "OCR button initialized";

    // 选项按钮
    m_pinOptionButton = new ToolButton(this);

    m_pinOptionButton->setObjectName(AC_SUBTOOLWIDGET_PIN_OPTION_BUT);
    m_pinOptionButton->setAccessibleName(AC_SUBTOOLWIDGET_PIN_OPTION_BUT);
    m_pinOptionButton->setIcon(QIcon::fromTheme("setting"));
    m_pinOptionButton->setIconSize(QSize(24, 24));
    m_pinOptionButton->setFixedSize(QSize(36, 36));
    m_pinOptionButton->setToolTip(tr("Options"));
    qCDebug(dsrApp) << "Options button initialized";

    m_saveLocalDirButton = new SaveButton();
    m_saveLocalDirButton->setCheckable(false);
    // 初始化时调用 updateSaveButtonTip 设置初始的 tooltip
    m_saveLocalDirButton->setSaveIcon(QIcon::fromTheme("save"));
    m_saveLocalDirButton->setListIcon(QIcon::fromTheme("dropdown"));
    // 监听保存按钮的悬停提示事件
    m_saveLocalDirButton->installEventFilter(this);
    m_pinOptionButton->setObjectName(AC_TOOLBARWIDGET_SAVE_LOACL_PIN_BUT);
    m_saveLocalDirButton->setAccessibleName(AC_TOOLBARWIDGET_SAVE_LOACL_PIN_BUT);
    
    // 创建并设置保存菜单管理器
    PinSaveMenuManager *pinSaveMenuManager = new PinSaveMenuManager(this);
    m_saveLocalDirButton->setOptionsMenu(pinSaveMenuManager->getMenu());
    
    connect(m_saveLocalDirButton, &SaveButton::clicked, this, &SubToolWidget::signalSaveToLocalButtonClicked);

    QActionGroup *t_formatGroup = new QActionGroup(this);
    qCDebug(dsrApp) << "Format action group created.";
    t_formatGroup->setExclusive(true);
    qCDebug(dsrApp) << "Action groups set to exclusive.";

    // 选项菜单
    m_optionMenu = new DMenu(this);
    qCDebug(dsrApp) << "Options menu created.";
    DFontSizeManager::instance()->bind(m_optionMenu, DFontSizeManager::T6);
    connect(m_optionMenu, &DMenu::aboutToShow, this, &SubToolWidget::updateOptionChecked);
    m_optionMenu->installEventFilter(this);  // 安装事件过滤器
    qCDebug(dsrApp) << "Options menu created";

    QAction *formatTitleAction = new QAction(m_optionMenu);
    QAction *pngAction = new QAction(m_optionMenu);
    QAction *jpgAction = new QAction(m_optionMenu);
    QAction *bmpAction = new QAction(m_optionMenu);
    qCDebug(dsrApp) << "Format actions created.";

    formatTitleAction->setDisabled(true);
    formatTitleAction->setText(tr("Format"));
    pngAction->setText(tr("PNG"));
    pngAction->setCheckable(true);
    jpgAction->setText(tr("JPG"));
    jpgAction->setCheckable(true);
    bmpAction->setText(tr("BMP"));
    bmpAction->setCheckable(true);

    t_formatGroup->addAction(pngAction);
    t_formatGroup->addAction(jpgAction);
    t_formatGroup->addAction(bmpAction);
    qCDebug(dsrApp) << "Format actions added to format group.";

    m_SaveFormatActions.insert(PNG, pngAction);
    m_SaveFormatActions.insert(JPG, jpgAction);
    m_SaveFormatActions.insert(BMP, bmpAction);
    qCDebug(dsrApp) << "Save format actions initialized";
    
    // 保存格式
    m_optionMenu->addAction(formatTitleAction);
    m_optionMenu->addAction(pngAction);
    m_optionMenu->addAction(jpgAction);
    m_optionMenu->addAction(bmpAction);
    
    m_pinOptionButton->setMenu(m_optionMenu);
    m_pinOptionButton->setPopupMode(QToolButton::InstantPopup);
    qCDebug(dsrApp) << "Option menu structure completed";

    connect(t_formatGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
    [ = ](QAction * t_act) {
        if (t_act == pngAction) {
            qCDebug(dsrApp) << "Selected format: PNG.";
            m_SaveInfo.second = PNG;
        } else if (t_act == jpgAction) {
            qCDebug(dsrApp) << "Selected format: JPG.";
            m_SaveInfo.second = JPG;
        } else if (t_act == bmpAction) {
            qCDebug(dsrApp) << "Selected format: BMP.";
            m_SaveInfo.second = BMP;
        }
        Settings::instance()->setSaveOption(m_SaveInfo);
        updateSaveButtonTip(); // 更新保存按钮的提示文本
    });

    m_saveSeperatorBeg = new DVerticalLine(this);
    m_saveSeperatorEnd = new DVerticalLine(this);
    m_saveSeperatorBeg->setFixedSize(QSize(3, 26));
    m_saveSeperatorEnd->setFixedSize(QSize(3, 26));

    //工具栏布局
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setSizeConstraint(QLayout::SetFixedSize);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(m_ocrButton, 0,  Qt::AlignCenter);
    hLayout->addWidget(m_saveSeperatorBeg, 0, Qt::AlignCenter);
    hLayout->addWidget(m_pinOptionButton, 0, Qt::AlignCenter);
    hLayout->addWidget(m_saveLocalDirButton, 0, Qt::AlignCenter);
    hLayout->addWidget(m_saveSeperatorEnd, 0, Qt::AlignCenter);
    m_shotSubTool->setLayout(hLayout);
    addWidget(m_shotSubTool);
    updateOptionChecked();
    qInfo() << __LINE__ << __FUNCTION__ << "贴图工具栏初始化完成";
}

void SubToolWidget::initChangeSaveToSpecialAction(const QString specialPath)
{
    m_changeSaveToSpecialPath->setText(tr("Change the path on save"));
    //根据字体大小计算字符串宽度，确定路径省略的长度
    // QFontMetrics tempFont(m_changeSaveToSpecialPath->font());
    // auto changeSaveToSpecialPathFontWidth = tempFont.boundingRect(m_changeSaveToSpecialPath->text()).width();
    
    // 处理路径显示逻辑为【/上级文件夹/本文件夹】，本文件夹限制10个字符
    QFileInfo fileInfo(specialPath);
    QString dirName = fileInfo.fileName();
    QString parentDir = fileInfo.dir().dirName();
    
    // 限制本文件夹名称为10个字符，超过则用省略号
    if (dirName.length() > 10) {
        dirName = dirName.left(7) + "...";
    }
    
    // 组合成【/上级文件夹/本文件夹】格式
    QString displayPath = QString("/%1/%2").arg(parentDir).arg(dirName);
    
    m_saveToSpecialPathAction->setText(displayPath);
    m_saveToSpecialPathAction->setToolTip(specialPath);
    m_saveToSpecialPathAction->setCheckable(true);
    m_saveToSpecialPathMenu->insertAction(m_changeSaveToSpecialPath, m_saveToSpecialPathAction);
    m_saveGroup->addAction(m_saveToSpecialPathAction);
    m_SavePathActions.insert(FOLDER, m_saveToSpecialPathAction);
}

/**
 * @brief 更新保存按钮的提示文本
 */
void SubToolWidget::updateSaveButtonTip()
{
    if (!m_saveLocalDirButton) {
        return;
    }
    
    // 使用实时配置生成安全的默认提示文案（事件过滤器会覆盖为更细粒度文案）
    QPair<int, int> saveInfo = Settings::instance()->getSaveOption();
    const int pathType = saveInfo.first;
    const QString specialPath = Settings::instance()->getSavePath();
    const bool hasHistoryPath = !specialPath.isEmpty() && QFileInfo::exists(specialPath);

    QString tipText;
    if (pathType == ASK) {
        tipText = tr("Save to local");
    } else if (pathType == FOLDER_CHANGE) {
        tipText = tr("Select a location when saving");
    } else if (pathType == FOLDER) {
        tipText = hasHistoryPath ? tr("Save to %1").arg(specialPath)
                                 : tr("Save to local");
    } else {
        tipText = tr("Save to local");
    }

    m_saveLocalDirButton->setToolTip(tipText);
}

// 获取保存信息
QPair<int, int> SubToolWidget::getSaveInfo()
{
    return m_SaveInfo;
}

// 选项按钮被点击
void SubToolWidget::onOptionButtonClicked()
{
    if (m_optionMenu->isHidden()) {
        // 显示菜单前，确保工具栏保持显示状态
        QWidget *toolbarWidget = qobject_cast<QWidget*>(parent());
        if (toolbarWidget) {
            toolbarWidget->show();
            toolbarWidget->activateWindow();
        }
        
        connect(m_optionMenu, &QMenu::aboutToHide, this, [this]() {
            QWidget *toolbarWidget = qobject_cast<QWidget*>(parent());
            if (toolbarWidget) {
                toolbarWidget->show();
                toolbarWidget->activateWindow();
            }
        }, Qt::SingleShotConnection);
        
        m_pinOptionButton->showMenu();
    } else {
        m_optionMenu->hide();
    }
}

//根据配置文件更新当前选中的选项
void SubToolWidget::updateOptionChecked()
{
    qInfo() << __LINE__ << __FUNCTION__ << "更新菜单选项";
    QPair<int, int> saveInfo = Settings::instance()->getSaveOption();
    qWarning() << "saveInfo: " << saveInfo;
    if (saveInfo.second != PNG && saveInfo.second != JPG && saveInfo.second != BMP) { //不存在保存格式的情况
        saveInfo.second = PNG; // 默认保存格式
        qCDebug(dsrApp) << "Invalid save format, defaulting to PNG";
    }
    //没有配置文件时，给定一个默认值
    if (saveInfo.first == 0 && saveInfo.second == 0) {
        // 首次使用时，默认选择"每次询问"选项
        m_SaveInfo.first = ASK;
        m_SaveInfo.second = PNG; // 默认保存格式
        qWarning(dsrApp) << "No configuration found, using defaults: ASK and PNG";
        // 保存默认设置到配置文件
        Settings::instance()->setSaveOption(m_SaveInfo);
    } else {
        m_SaveInfo = saveInfo;
    }
    qWarning() << "m_SaveInfo: " << m_SaveInfo;
    
    // 设置保存格式
    if (m_SaveFormatActions.contains(m_SaveInfo.second)) {
        m_SaveFormatActions.value(m_SaveInfo.second)->setChecked(true);
    }
    
    // 更新保存按钮的提示文本
    updateSaveButtonTip();
    
    qCDebug(dsrApp) << "Menu options update completed";
}

bool SubToolWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_saveLocalDirButton) {
        if (event->type() == QEvent::ToolTip) {
            QPair<int, int> saveInfo = Settings::instance()->getSaveOption();
            const QString specialPath = Settings::instance()->getSavePath();
            const bool hasHistoryPath = !specialPath.isEmpty() && QFileInfo::exists(specialPath);

            QString tipText;
            switch (saveInfo.first) {
            case ASK:
                tipText = tr("Save to local");
                break;
            case FOLDER:
                if (hasHistoryPath) {
                    tipText = tr("Save to %1").arg(specialPath);
                } else {
                    tipText = tr("Save to local");
                }
                break;
            case FOLDER_CHANGE:
                // 菜单选中"保存时设置位置" → 根据是否有历史路径决定文案
                tipText = hasHistoryPath ? tr("Update the location when saving")
                                         : tr("Select a location when saving");
                break;
            default:
                tipText = tr("Save to local");
                break;
            }
            QHelpEvent *he = static_cast<QHelpEvent *>(event);
            QToolTip::showText(he->globalPos(), tipText, m_saveLocalDirButton);

            return true;
        }
    }

    if (watched == m_optionMenu) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            QAction *action = static_cast<DMenu *>(watched)->actionAt(mouseEvent->pos());
            if (action) {
                if (action->menu() && action->menu()->title() == tr("Specified Location")) {
                    // 检查子菜单中是否有选中项
                    bool hasCheckedChild = false;
                    
                    // 检查桌面选项
                    if (m_SavePathActions.contains(DESKTOP) && m_SavePathActions.value(DESKTOP) && 
                        m_SavePathActions.value(DESKTOP)->isChecked()) {
                        m_SaveInfo.first = DESKTOP;
                        hasCheckedChild = true;
                    } 
                    // 检查图片选项
                    else if (m_SavePathActions.contains(PICTURES) && m_SavePathActions.value(PICTURES) && 
                             m_SavePathActions.value(PICTURES)->isChecked()) {
                        m_SaveInfo.first = PICTURES;
                        hasCheckedChild = true;
                    } 
                    // 检查自定义位置选项
                    else if (m_saveToSpecialPathMenu && m_saveToSpecialPathMenu->menuAction()->isChecked()) {
                        if (m_saveToSpecialPathAction && m_saveToSpecialPathAction->isChecked()) {
                            m_SaveInfo.first = FOLDER;
                            hasCheckedChild = true;
                        } else if (m_changeSaveToSpecialPath && m_changeSaveToSpecialPath->isChecked()) {
                            m_SaveInfo.first = FOLDER_CHANGE;
                            Settings::instance()->setIsChangeSavePath(true);
                            hasCheckedChild = true;
                        }
                    }
                    
                    if (hasCheckedChild) {
                        action->setChecked(true);
                        m_askEveryTimeAction->setChecked(false);
                        
                        Settings::instance()->setSaveOption(m_SaveInfo);
                        updateSaveButtonTip();
                    } else {
                        action->setChecked(false);
                        return true;
                    }
                    
                    return false;
                }
                
                action->activate(QAction::Trigger);
                static_cast<DMenu *>(watched)->hide();
                return true;
            }
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            QAction *action = static_cast<DMenu *>(watched)->actionAt(mouseEvent->pos());
            if (action) {
                return true;
            }
        }
    }
    
    if (watched == m_saveToSpecialPathMenu) {
        if(event->type() == QEvent::ToolTip){
            QHelpEvent* he = dynamic_cast<QHelpEvent*>(event);
            QAction *action = static_cast<QMenu *>(watched)->actionAt(he->pos());
            if (action != nullptr) {
                if (action == m_saveToSpecialPathAction) {
                    QToolTip::showText(he->globalPos(), action->toolTip(), this);
                }
                if (action == m_changeSaveToSpecialPath) {
                    QToolTip::hideText();
                }
            }
        }
    }
    return false;
}
