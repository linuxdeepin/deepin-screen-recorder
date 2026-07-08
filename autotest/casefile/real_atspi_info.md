# deepin-screen-recorder 真实 AT-SPI 信息（实测）

## 应用启动方式

deepin-screen-recorder 是快捷键激活的应用，不通过 session_start 启动。

- 截图: `keyboard_hot_key` keys: `ctrl,alt,a`
- 录屏: `keyboard_hot_key` keys: `ctrl,alt,r`
- 退出: `keyboard_press` keys: `Escape`

## 重要: 必须设置 QT_ACCESSIBILITY=1

启动应用前必须设置环境变量 `QT_ACCESSIBILITY=1`，否则应用不暴露子控件 AT-SPI 信息。
截图模式下只有 8 个基础节点，**框选区域后**才出现工具栏（163 个节点）。

YAML 中不需要也不应该有 setup 步骤，直接用快捷键激活即可。

## 截图模式（框选区域后）真实 AT-SPI 树

### 工具栏按钮（check box 和 button）
| AT-SPI name | role | 功能 | 快捷键 |
|------------|------|------|--------|
| keyboard_button | check box | 显示按键 | K |
| camera_button | check box | 开启摄像头 | C |
| shot_button | button | 截图 | - |
| record_option_but | push button menu | 录屏选项(F3) | F3 |
| gio_button | check box | 几何图形 | R |
| line_button | check box | 直线 | L |
| line_button (第2个) | check box | 直线(箭头) | X |
| pen_button | check box | 画笔 | P |
| mosaic_button | check box | 模糊 | B |
| text_button | check box | 文字 | T |
| scrollshot_button | button | 滚动截图 | Alt+I |
| orc_button | button | 文字识别 | Alt+O |
| pinscreenshots_button | button | 贴图 | Alt+P |
| ai_assistant_button | check box | AI截图 | A |
| undo_button | button | 撤销 | Ctrl+Z |
| recorder_button | button | 录屏 | - |
| shot_option_but | push button menu | 截图选项(F3) | F3 |
| save_local_button | button | 保存到图片 | - |
| toolbar_close_button | button | 关闭 | Esc |

### 录屏选项菜单（record_option_but 点击展开）
| 路径 | 功能 |
|------|------|
| 格式/GIF | 选择GIF格式 |
| 格式/MP4 | 选择MP4格式 |
| 格式/MKV | 选择MKV格式 |
| 帧数/5 fps | 5帧 |
| 帧数/10 fps | 10帧 |
| 帧数/20 fps | 20帧 |
| 帧数/24 fps | 24帧 |
| 帧数/30 fps | 30帧 |
| 声音/麦克风 | 麦克风 |
| 声音/系统音频 | 系统音频 |
| 选项/显示光标 | 显示光标 |
| 选项/显示鼠标点击 | 显示鼠标点击 |
| 保存方式/桌面 | 保存到桌面 |
| 保存方式/视频 | 保存到视频 |

### 截图选项菜单（shot_option_but 点击展开）
| 路径 | 功能 |
|------|------|
| 边框效果/无边框 | 无边框 |
| 边框效果/投影 | 投影效果 |
| 边框效果/外边框 | 外边框 |
| 边框效果/样机 | 样机边框 |
| 格式/PNG | PNG格式 |
| 格式/JPG | JPG格式 |
| 格式/BMP | BMP格式 |
| 选项/显示光标 | 显示光标 |

### 保存按钮菜单（save_local_button 点击展开）
| 路径 | 功能 |
|------|------|
| 每次询问 | 每次询问保存位置 |
| 指定位置/桌面 | 指定桌面 |
| 指定位置/图片 | 指定图片 |
| 指定位置/保存时选择位置 | 弹窗选择 |
| 指定位置/保存时更新位置 | 保存时更新 |

## 实际截图录制流程

### 截图流程
1. `keyboard_hot_key` keys: `ctrl,alt,a` wait: 1.0
2. `mouse_drag` 框选区域（start/end 坐标）
3. 等待工具栏出现（wait: 1.0）
4. 工具操作（如 element_action ref: rect_button do: click）
5. `keyboard_press` keys: `Enter` 确认/保存
6. 断言截图已保存

### 录屏流程
1. `keyboard_hot_key` keys: `ctrl,alt,r` wait: 1.0
2. `mouse_drag` 框选区域
3. 等待工具栏出现（wait: 1.0）
4. 点击开始或 `keyboard_press` keys: `Return` 倒计时开始
5. 录制中操作（可切换摄像头/按键显示等）
6. `keyboard_hot_key` keys: `ctrl,alt,r` 停止
7. 断言视频已保存

## 关键：QTextEdit 子控件

文字工具点击后会创建 QPlainTextEdit，其 AT-SPI name 为空字符串。
需要通过坐标操作而非 element_action。

## 坐标栏 tooltip 验证

用例描述中 label 节点的内容对应按钮的 tooltip：
- '显示按键 K'
- '开启摄像头 C'
- '截图'
- '设置 F3'
- '几何工具 (R)\n按住Shift可画正方形/正圆'
- '直线（L）\n按住Shift可画垂直线和水平线'
- '箭头（X）\n按住Shift可画垂直和水平箭头'
- '画笔 P'
- '模糊 B'
- '文字 T'
- '滚动截图 Alt+I'
- '识别文字 Alt+O'
- '贴图 Alt+P'
- 'AI截图 (A)'
- '撤销 Ctrl+Z'
- '录屏'
- '设置 F3'
- '保存到图片'
- '关闭 Esc'
- '复制到剪贴板 Enter'

## 错误排查

如果元素找不到，检查：
1. 是否先框选了区域（工具栏只在有选区时才出现）
2. 是否按了正确的快捷键
3. 是否等待足够时间让界面更新
