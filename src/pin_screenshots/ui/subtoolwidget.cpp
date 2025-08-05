// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "subtoolwidget.h"
#include "accessibility/acTextDefine.h"
#include "settings.h"
#include "../../utils/log.h"

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
    qCDebug(dsrApp) << "Options button created.";
    m_pinOptionButton->setCheckable(false);
    m_pinOptionButton->setFlat(false);
    m_pinOptionButton->setHoverState(false);
    DPalette pa = m_pinOptionButton->palette();
    DGuiApplicationHelper::ColorType t_type = DGuiApplicationHelper::instance()->themeType();
    if (t_type == DGuiApplicationHelper::ColorType::LightType) {
        qCDebug(dsrApp) << "Theme type is LightType, setting palette colors.";
        pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
        pa.setColor(DPalette::Dark, QColor(192, 192, 192, 255));
        pa.setColor(DPalette::Light, QColor(192, 192, 192, 255));
    } else {
        qCDebug(dsrApp) << "Theme type is DarkType, setting palette colors.";
        pa.setColor(DPalette::ButtonText, QColor(228, 228, 228, 255));
        pa.setColor(DPalette::Dark, QColor(64, 64, 64, 255));
        pa.setColor(DPalette::Light, QColor(64, 64, 64, 255));
    }
    m_pinOptionButton->setPalette(pa);
    m_pinOptionButton->setFocusPolicy(Qt::NoFocus);
    m_pinOptionButton->setObjectName(AC_SUBTOOLWIDGET_PIN_OPTION_BUT);
    m_pinOptionButton->setAccessibleName(AC_SUBTOOLWIDGET_PIN_OPTION_BUT);
    DFontSizeManager::instance()->bind(m_pinOptionButton, DFontSizeManager::T6);
    m_pinOptionButton->setText(tr("Options"));
    m_pinOptionButton->setMinimumSize(QSize(60, 36));
    m_pinOptionButton->setToolTip(tr("Options"));
    qCDebug(dsrApp) << "Options button initialized";

    m_saveLocalDirButton = new ToolButton();
    m_saveLocalDirButton->setCheckable(false);
    m_saveLocalDirButton->setIconSize(QSize(32, 32));
    // 初始化时调用 updateSaveButtonTip 设置初始的 tooltip
    m_saveLocalDirButton->setIcon(QIcon(":/icons/deepin/builtin/texts/save.svg"));
    m_pinOptionButton->setObjectName(AC_TOOLBARWIDGET_SAVE_LOACL_PIN_BUT);
    m_saveLocalDirButton->setAccessibleName(AC_TOOLBARWIDGET_SAVE_LOACL_PIN_BUT);
    m_saveLocalDirButton->setFixedSize(QSize(32, 32));
    connect(m_saveLocalDirButton, &ToolButton::clicked, this, &SubToolWidget::signalSaveToLocalButtonClicked);

    m_saveGroup = new QActionGroup(this);
    qCDebug(dsrApp) << "Save action group created.";
    QActionGroup *t_formatGroup = new QActionGroup(this);
    qCDebug(dsrApp) << "Format action group created.";
    m_saveGroup->setExclusive(true);
    t_formatGroup->setExclusive(true);
    qCDebug(dsrApp) << "Action groups set to exclusive.";

    // 选项菜单
    m_optionMenu = new DMenu(this);
    qCDebug(dsrApp) << "Options menu created.";
    DFontSizeManager::instance()->bind(m_optionMenu, DFontSizeManager::T6);
    connect(m_optionMenu, &DMenu::aboutToShow, this, &SubToolWidget::updateOptionChecked);
    m_optionMenu->installEventFilter(this);  // 安装事件过滤器
    qCDebug(dsrApp) << "Options menu created";

    // 创建每次询问选项
    m_askEveryTimeAction = new QAction(tr("Each inquiry"), m_optionMenu);
    m_askEveryTimeAction->setCheckable(true);

    // 保存位置标题
    QAction *saveTitleAction = new QAction(tr("Save to"), m_optionMenu);
    saveTitleAction->setDisabled(true);
    
    // 创建剪贴板选项
    // QAction *saveToClipAction = new QAction(tr("Clipboard"), m_optionMenu);
    // saveToClipAction->setCheckable(true);
    
    // 创建指定位置子菜单
    DMenu *specifiedLocationMenu = new DMenu(m_optionMenu);
    specifiedLocationMenu->setTitle(tr("Specified Location"));
    specifiedLocationMenu->setToolTipsVisible(true);
    specifiedLocationMenu->menuAction()->setCheckable(true);
    DFontSizeManager::instance()->bind(specifiedLocationMenu, DFontSizeManager::T8);
    
    // 指定位置子菜单项
    QAction *saveToDesktopAction = new QAction(tr("Desktop"), specifiedLocationMenu);
    saveToDesktopAction->setCheckable(true);
    
    QAction *saveToPictureAction = new QAction(tr("Pictures"), specifiedLocationMenu);
    saveToPictureAction->setCheckable(true);
    
    // 创建自定义位置子菜单
    m_saveToSpecialPathMenu = new DMenu(specifiedLocationMenu);
    m_saveToSpecialPathMenu->setTitle(tr("Custom Location"));
    m_saveToSpecialPathMenu->setToolTipsVisible(true);
    m_saveToSpecialPathMenu->menuAction()->setCheckable(true);
    DFontSizeManager::instance()->bind(m_saveToSpecialPathMenu, DFontSizeManager::T8);
    
    QString specialPath = Settings::instance()->getSavePath();
    qCDebug(dsrApp) << "Special save path menu created";

    // 上次保存位置
    m_saveToSpecialPathAction = new QAction(m_saveToSpecialPathMenu);
    m_saveToSpecialPathAction->setCheckable(true);
    
    // 保存时更新位置
    m_changeSaveToSpecialPath = new QAction(m_saveToSpecialPathMenu);
    m_changeSaveToSpecialPath->setCheckable(true);
    
    // 如果有保存路径，显示上次保存位置
    if (specialPath.isEmpty() || !QFileInfo::exists(specialPath)) {
        qCDebug(dsrApp) << "Specified path does not exist or is empty.";
        m_changeSaveToSpecialPath->setText(tr("Set a path on save"));
        // 默认选中"每次询问"选项
        m_askEveryTimeAction->setChecked(true);
        // 取消选中指定位置菜单和自定义位置子菜单
        specifiedLocationMenu->menuAction()->setChecked(false);
        m_saveToSpecialPathMenu->menuAction()->setChecked(false);
    } else {
        qCDebug(dsrApp) << "Specified path exists.";
        initChangeSaveToSpecialAction(specialPath);
        m_saveToSpecialPathMenu->addAction(m_saveToSpecialPathAction);
        // 默认选中历史保存路径
        m_saveToSpecialPathAction->setChecked(true);
        // 同时选中指定位置菜单和自定义位置子菜单
        specifiedLocationMenu->menuAction()->setChecked(true);
        m_saveToSpecialPathMenu->menuAction()->setChecked(true);
    }
    
    m_saveToSpecialPathMenu->addAction(m_changeSaveToSpecialPath);
    m_SavePathActions.insert(FOLDER_CHANGE, m_changeSaveToSpecialPath);
    qCDebug(dsrApp) << "Save to special path action initialized.";

    // 创建指定位置子菜单的选项组
    QActionGroup *specifiedLocationGroup = new QActionGroup(this);
    specifiedLocationGroup->setExclusive(true);
    specifiedLocationGroup->addAction(saveToDesktopAction);
    specifiedLocationGroup->addAction(saveToPictureAction);
    specifiedLocationGroup->addAction(m_saveToSpecialPathMenu->menuAction());
    
    // 创建自定义位置子菜单的选项组
    QActionGroup *customLocationGroup = new QActionGroup(this);
    customLocationGroup->setExclusive(true);
    if (!specialPath.isEmpty() && QFileInfo::exists(specialPath)) {
        customLocationGroup->addAction(m_saveToSpecialPathAction);
    }
    customLocationGroup->addAction(m_changeSaveToSpecialPath);

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

    // 添加到保存选项组
    // m_saveGroup->addAction(saveToClipAction);
    m_saveGroup->addAction(specifiedLocationMenu->menuAction());
    m_saveGroup->addAction(m_askEveryTimeAction);
    
    // m_SavePathActions.insert(CLIPBOARD, saveToClipAction);
    m_SavePathActions.insert(DESKTOP, saveToDesktopAction);
    m_SavePathActions.insert(PICTURES, saveToPictureAction);
    // 添加每次询问选项到保存路径映射
    m_SavePathActions.insert(ASK, m_askEveryTimeAction);
    qCDebug(dsrApp) << "Save path actions initialized";

    // 添加到指定位置子菜单
    specifiedLocationMenu->addAction(saveToDesktopAction);
    specifiedLocationMenu->addAction(saveToPictureAction);
    specifiedLocationMenu->addMenu(m_saveToSpecialPathMenu);

    // 添加到主菜单
    m_optionMenu->addAction(saveTitleAction);
    // m_optionMenu->addAction(saveToClipAction);
    m_optionMenu->addMenu(specifiedLocationMenu);
    m_optionMenu->addAction(m_askEveryTimeAction);
    m_optionMenu->addSeparator();
    
    // 保存格式
    m_optionMenu->addAction(formatTitleAction);
    m_optionMenu->addAction(pngAction);
    m_optionMenu->addAction(jpgAction);
    m_optionMenu->addAction(bmpAction);
    
    m_pinOptionButton->setMenu(m_optionMenu);
    qCDebug(dsrApp) << "Option menu structure completed";

    // 保存选项组的信号连接
    connect(m_saveGroup, QOverload<QAction *>::of(&QActionGroup::triggered), [ = ](QAction * t_act) {
        if (t_act == specifiedLocationMenu->menuAction()) {
            // 当点击指定位置菜单时，确保该菜单被选中
            specifiedLocationMenu->menuAction()->setChecked(true);
            
            // 取消"每次询问"选项的选中状态
            if (m_askEveryTimeAction && m_askEveryTimeAction->isChecked()) {
                m_askEveryTimeAction->setChecked(false);
            }
            
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
            
            // 如果没有子菜单项被选中，则从配置文件中读取上次的选择
            if (!hasCheckedChild) {
                // 从配置文件中读取上次的选择
                QPair<int, int> saveInfo = Settings::instance()->getSaveOption();
                
                // 如果上次选择不是"每次询问"，则恢复上次的选择
                if (saveInfo.first != ASK && 
                    (saveInfo.first == DESKTOP || saveInfo.first == PICTURES || 
                     saveInfo.first == FOLDER || saveInfo.first == FOLDER_CHANGE)) {
                    // 恢复上次的选择
                    m_SaveInfo.first = saveInfo.first;
                    
                    // 根据保存类型设置相应的选中状态
                    if (m_SavePathActions.contains(m_SaveInfo.first) && 
                        m_SavePathActions.value(m_SaveInfo.first) != nullptr) {
                        m_SavePathActions.value(m_SaveInfo.first)->setChecked(true);
                        
                        // 如果是自定义位置，还需要选中自定义位置菜单
                        if (m_SaveInfo.first == FOLDER || m_SaveInfo.first == FOLDER_CHANGE) {
                            m_saveToSpecialPathMenu->menuAction()->setChecked(true);
                        }
                    }
                    
                    // 设置是否更改保存路径的标志
                    if (m_SaveInfo.first == FOLDER_CHANGE) {
                        Settings::instance()->setIsChangeSavePath(true);
                    } else {
                        Settings::instance()->setIsChangeSavePath(false);
                    }
                } else {
                    // 如果配置文件中没有有效的选择，则使用默认选项
                    // 优先选择之前用户保存过的路径
                    QString specialPath = Settings::instance()->getSavePath();
                    if (!specialPath.isEmpty() && QFileInfo::exists(specialPath) && m_saveToSpecialPathAction) {
                        // 选中自定义位置菜单
                        m_saveToSpecialPathMenu->menuAction()->setChecked(true);
                        // 选中历史路径选项
                        m_saveToSpecialPathAction->setChecked(true);
                        m_SaveInfo.first = FOLDER;
                        Settings::instance()->setIsChangeSavePath(false);
                    } else {
                        // 如果没有历史路径，则选中自定义位置菜单下的"保存时选择位置"选项
                        m_saveToSpecialPathMenu->menuAction()->setChecked(true);
                        m_changeSaveToSpecialPath->setChecked(true);
                        m_SaveInfo.first = FOLDER_CHANGE;
                        Settings::instance()->setIsChangeSavePath(true);
                    }
                }
            }
        } else if (t_act == m_askEveryTimeAction) {
            qCDebug(dsrApp) << "Selected: Ask every time.";
            m_SaveInfo.first = ASK;
            
            // 取消"指定位置"菜单的选中状态，但保留子菜单的选中状态
            for (auto action : m_optionMenu->actions()) {
                if (action->menu() && action->menu()->title() == tr("Specified Location")) {
                    action->setChecked(false);
                    break;
                }
            }
            m_optionMenu->hide();
        }
        
        Settings::instance()->setSaveOption(m_SaveInfo);
        qCDebug(dsrApp) << "Save option set in settings.";
        updateSaveButtonTip(); // 更新保存按钮的提示文本
    });

    // 指定位置子菜单的信号连接
    connect(specifiedLocationGroup, QOverload<QAction *>::of(&QActionGroup::triggered), [ = ](QAction * t_act) {
        // 确保指定位置菜单被选中
        specifiedLocationMenu->menuAction()->setChecked(true);
        
        // 取消"每次询问"选项的选中状态
        if (m_askEveryTimeAction && m_askEveryTimeAction->isChecked()) {
            m_askEveryTimeAction->setChecked(false);
        }
        
        if (t_act == saveToDesktopAction) {
            qCDebug(dsrApp) << "Selected: Save to Desktop.";
            m_SaveInfo.first = DESKTOP;
            Settings::instance()->setIsChangeSavePath(false);
        } else if (t_act == saveToPictureAction) {
            qCDebug(dsrApp) << "Selected: Save to Pictures.";
            m_SaveInfo.first = PICTURES;
            Settings::instance()->setIsChangeSavePath(false);
        } else if (t_act == m_saveToSpecialPathMenu->menuAction()) {
            // 当点击自定义位置菜单时，确保该菜单被选中
            m_saveToSpecialPathMenu->menuAction()->setChecked(true);
            
            // 根据子菜单选中项设置保存位置
            if (m_saveToSpecialPathAction && m_saveToSpecialPathAction->isChecked()) {
                m_SaveInfo.first = FOLDER;
                Settings::instance()->setIsChangeSavePath(false);
            } else if (m_changeSaveToSpecialPath && m_changeSaveToSpecialPath->isChecked()) {
                m_SaveInfo.first = FOLDER_CHANGE;
                Settings::instance()->setIsChangeSavePath(true);
            } else {
                // 如果没有子项被选中，则默认选中"保存时选择位置"
                m_SaveInfo.first = FOLDER_CHANGE;
                Settings::instance()->setIsChangeSavePath(true);
                m_changeSaveToSpecialPath->setChecked(true);
            }
        }
        
        Settings::instance()->setSaveOption(m_SaveInfo);
        updateSaveButtonTip(); // 更新保存按钮的提示文本
    });

    // 自定义位置子菜单的信号连接
    connect(customLocationGroup, QOverload<QAction *>::of(&QActionGroup::triggered), [ = ](QAction * t_act) {
        // 选中指定位置菜单和自定义位置子菜单
        specifiedLocationMenu->menuAction()->setChecked(true);
        m_saveToSpecialPathMenu->menuAction()->setChecked(true);
        
        // 取消"每次询问"选项的选中状态
        if (m_askEveryTimeAction && m_askEveryTimeAction->isChecked()) {
            m_askEveryTimeAction->setChecked(false);
        }
        
        if (t_act == m_changeSaveToSpecialPath) {
            qCDebug(dsrApp) << "Selected: Set or change specified save location.";
            m_SaveInfo.first = FOLDER_CHANGE;
            Settings::instance()->setIsChangeSavePath(true);
        } else if (t_act == m_saveToSpecialPathAction) {
            qCDebug(dsrApp) << "Selected: Save to specified location (FOLDER).";
            m_SaveInfo.first = FOLDER;
            Settings::instance()->setIsChangeSavePath(false);
        }
        
        Settings::instance()->setSaveOption(m_SaveInfo);
        updateSaveButtonTip(); // 更新保存按钮的提示文本
    });

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
    m_saveSeperatorBeg->setFixedSize(QSize(3, 30));
    m_saveSeperatorEnd->setFixedSize(QSize(3, 30));

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
    
    // 根据当前保存选项设置不同的提示文本
    switch (m_SaveInfo.first) {
        case ASK:
            // 每次询问
            m_saveLocalDirButton->setToolTip(tr("Save to local"));
            break;
        case DESKTOP:
            // 保存到桌面
            m_saveLocalDirButton->setToolTip(tr("Save to desktop"));
            break;
        case PICTURES:
            // 保存到图片
            m_saveLocalDirButton->setToolTip(tr("Save to pictures"));
            break;
        case FOLDER_CHANGE:
            // 保存时选择位置
            m_saveLocalDirButton->setToolTip(m_changeSaveToSpecialPath->text());
            break;
        case FOLDER:
            // 保存到指定位置
            {
                QString specialPath = Settings::instance()->getSavePath();
                if (!specialPath.isEmpty() && QFileInfo::exists(specialPath)) {
                    m_saveLocalDirButton->setToolTip(tr("Save to %1").arg(specialPath));
                } else {
                    m_saveLocalDirButton->setToolTip(tr("Save to local"));
                }
            }
            break;
        default:
            m_saveLocalDirButton->setToolTip(tr("Save to local"));
            break;
    }
}

// 获取保存信息
QPair<int, int> SubToolWidget::getSaveInfo()
{
    return m_SaveInfo;
}

// 选项按钮被点击
void SubToolWidget::onOptionButtonClicked()
{
    if (m_optionMenu->isHidden())
        m_pinOptionButton->showMenu();
    else
        m_optionMenu->hide();
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
    
    // 获取指定位置菜单
    QAction *specifiedLocationAction = nullptr;
    for (auto action : m_optionMenu->actions()) {
        if (action->menu() && action->menu()->title() == tr("Specified Location")) {
            specifiedLocationAction = action;
            break;
        }
    }
    
    // 根据当前保存选项设置顶层菜单项的选中状态
    // 注意：不要清除子菜单的选中状态，以便在切换回"指定位置"时能保留之前的选择
    if (m_SaveInfo.first == ASK) {
        // 如果是"每次询问"，则选中该选项，取消"指定位置"的选中状态
        if (m_askEveryTimeAction) {
            m_askEveryTimeAction->setChecked(true);
        }
        if (specifiedLocationAction) {
            specifiedLocationAction->setChecked(false);
        }
    } else if (m_SaveInfo.first == DESKTOP || m_SaveInfo.first == PICTURES || 
               m_SaveInfo.first == FOLDER || m_SaveInfo.first == FOLDER_CHANGE) {
        // 如果是"指定位置"下的选项，则选中"指定位置"菜单
        if (specifiedLocationAction) {
            specifiedLocationAction->setChecked(true);
        }
        if (m_askEveryTimeAction) {
            m_askEveryTimeAction->setChecked(false);
        }
        
        // 根据具体选项选中相应的子菜单项
        if (m_SavePathActions.contains(m_SaveInfo.first) && m_SavePathActions.value(m_SaveInfo.first) != nullptr) {
            m_SavePathActions.value(m_SaveInfo.first)->setChecked(true);
            
            // 如果是自定义位置，还需要选中自定义位置菜单
            if (m_SaveInfo.first == FOLDER || m_SaveInfo.first == FOLDER_CHANGE) {
                if (m_saveToSpecialPathMenu) {
                    m_saveToSpecialPathMenu->menuAction()->setChecked(true);
                }
            }
        }
    }
    
    // 处理自定义位置的特殊情况
    if (m_SaveInfo.first == FOLDER || m_SaveInfo.first == FOLDER_CHANGE) {
        QString specialPath = Settings::instance()->getSavePath();
        
        // 判断是否存在历史路径
        if (specialPath.isEmpty() || !QFileInfo::exists(specialPath)) {
            m_SaveInfo.first = FOLDER_CHANGE;
            qCDebug(dsrApp) << "Special path not found, switching to folder change mode";
            
            // 确保"保存时选择位置"选项被选中
            if (m_changeSaveToSpecialPath) {
                m_changeSaveToSpecialPath->setChecked(true);
            }
        } else {
            // 如果存在历史路径，则根据是否更改保存路径的标志决定选中哪个选项
            if (Settings::instance()->getIsChangeSavePath()) {
                m_SaveInfo.first = FOLDER_CHANGE;
                
                if (m_changeSaveToSpecialPath) {
                    m_changeSaveToSpecialPath->setChecked(true);
                }
                if (m_saveToSpecialPathAction) {
                    m_saveToSpecialPathAction->setChecked(false);
                }
            } else {
                m_SaveInfo.first = FOLDER;
                
                // 确保历史路径选项存在
                if (!m_SavePathActions.contains(FOLDER) || m_SavePathActions.value(FOLDER) == nullptr) {
                    initChangeSaveToSpecialAction(specialPath);
                } else {
                    // 更新历史路径的显示
                    QFileInfo fileInfo(specialPath);
                    QString dirName = fileInfo.fileName();
                    QString parentDir = fileInfo.dir().dirName();

                    if (dirName.length() > 10) {
                        dirName = dirName.left(7) + "...";
                    }

                    // 组合成【/上级文件夹/本文件夹】格式
                    QString displayPath = QString("/%1/%2").arg(parentDir).arg(dirName);

                    m_saveToSpecialPathAction->setText(displayPath);
                    m_saveToSpecialPathAction->setToolTip(specialPath);
                }
                
                // 选中历史路径选项
                if (m_saveToSpecialPathAction) {
                    m_saveToSpecialPathAction->setChecked(true);
                }
                if (m_changeSaveToSpecialPath) {
                    m_changeSaveToSpecialPath->setChecked(false);
                }
            }
        }
    }
    
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
    if (watched == m_optionMenu) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            QAction *action = static_cast<DMenu *>(watched)->actionAt(mouseEvent->pos());
            if (action) {
                // 特别处理"指定位置"菜单项
                if (action->menu() && action->menu()->title() == tr("Specified Location")) {
                    // 首先确保该菜单项被选中
                    action->setChecked(true);
                    
                    // 取消"每次询问"选项的选中状态
                    if (m_askEveryTimeAction && m_askEveryTimeAction->isChecked()) {
                        m_askEveryTimeAction->setChecked(false);
                    }
                    
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
                    
                    // 如果没有子菜单项被选中，则从配置文件中读取上次的选择
                    if (!hasCheckedChild) {
                        // 从配置文件中读取上次的选择
                        QPair<int, int> saveInfo = Settings::instance()->getSaveOption();
                        
                        // 如果上次选择不是"每次询问"，则恢复上次的选择
                        if (saveInfo.first != ASK && 
                            (saveInfo.first == DESKTOP || saveInfo.first == PICTURES || 
                             saveInfo.first == FOLDER || saveInfo.first == FOLDER_CHANGE)) {
                            // 恢复上次的选择
                            m_SaveInfo.first = saveInfo.first;
                            
                            // 根据保存类型设置相应的选中状态
                            if (m_SavePathActions.contains(m_SaveInfo.first) && 
                                m_SavePathActions.value(m_SaveInfo.first) != nullptr) {
                                m_SavePathActions.value(m_SaveInfo.first)->setChecked(true);
                                
                                // 如果是自定义位置，还需要选中自定义位置菜单
                                if (m_SaveInfo.first == FOLDER || m_SaveInfo.first == FOLDER_CHANGE) {
                                    m_saveToSpecialPathMenu->menuAction()->setChecked(true);
                                }
                            }
                            
                            // 设置是否更改保存路径的标志
                            if (m_SaveInfo.first == FOLDER_CHANGE) {
                                Settings::instance()->setIsChangeSavePath(true);
                            } else {
                                Settings::instance()->setIsChangeSavePath(false);
                            }
                        } else {
                            // 如果配置文件中没有有效的选择，则使用默认选项
                            // 优先选择之前用户保存过的路径
                            QString specialPath = Settings::instance()->getSavePath();
                            if (!specialPath.isEmpty() && QFileInfo::exists(specialPath)) {
                                m_SaveInfo.first = FOLDER;
                                Settings::instance()->setIsChangeSavePath(false);
                                
                                // 选中自定义位置菜单
                                m_saveToSpecialPathMenu->menuAction()->setChecked(true);
                                // 选中历史路径选项
                                if (m_saveToSpecialPathAction) {
                                    m_saveToSpecialPathAction->setChecked(true);
                                }
                            } else {
                                m_SaveInfo.first = FOLDER_CHANGE;
                                Settings::instance()->setIsChangeSavePath(true);
                                
                                // 选中自定义位置菜单下的"保存时选择位置"选项
                                m_saveToSpecialPathMenu->menuAction()->setChecked(true);
                                if (m_changeSaveToSpecialPath) {
                                    m_changeSaveToSpecialPath->setChecked(true);
                                }
                            }
                        }
                    }
                    
                    // 更新保存设置
                    Settings::instance()->setSaveOption(m_SaveInfo);
                    updateSaveButtonTip();
                    
                    // 允许事件继续传播，以便菜单能够正常展开
                    return false;
                }
                
                // 对其他菜单项，直接触发其 triggered 信号
                action->activate(QAction::Trigger);
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
