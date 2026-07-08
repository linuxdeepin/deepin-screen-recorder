# deepin-screen-recorder YouQu 自动化测试报告

> 测试时间: 2026-07-07
> 测试框架: youqu-ai
> 被测应用: /usr/bin/deepin-screen-recorder (系统安装版)
> 执行命令: `youqu run`
> 执行耗时: 310.71s (5分10秒)

## 1. 测试结果总览

| 指标 | 数值 |
|------|------|
| 收集用例总数 | 316 |
| 通过 | 149 (47.2%) |
| 失败 | 167 (52.8%) |
| 错误(收集阶段) | 0 |

## 2. 失败原因分类

| # | 失败原因 | 数量 | 占比 | 性质 |
|---|----------|------|------|------|
| 1 | ApplicationStartError: 应用程序未启动 | 130 | 77.8% | 连锁失败 |
| 2 | unknown action: exec/for_loop/shell 不支持 | 14 | 8.4% | 用例格式问题 |
| 3 | ElementNotFound: 未找到UI元素 | 6 | 3.6% | AT-SPI 兼容问题 |
| 4 | DBus 接口不存在 | 2 | 1.2% | 环境依赖问题 |
| 5 | 其他 | 15 | 9.0% | 连锁 ApplicationStartError |
| | **合计** | **167** | **100%** | |

## 3. 失败原因详细分析

### 3.1 ApplicationStartError: 应用程序未启动 (130个)

**根因分析**: deepin-screen-recorder 在截图模式下按 Escape 退出后，应用进程会完全退出，而非回到待命状态。后续用例通过 `keyboard_hot_key: ctrl,alt,a` 唤起截图时，框架先检查应用进程是否存在，发现进程不存在则直接抛出 `ApplicationStartError`，导致从失败用例开始后续所有用例全部连锁失败。

**影响范围**: 这是本次测试最主要的失败原因。一旦某个截图类用例失败导致应用退出，后续所有需要唤起截图的用例全部连锁失败。因此实际独立失败的用例远少于 130 个。

**涉及模块**:

| 模块 | 失败数 |
|------|--------|
| ocr | 19 |
| pinscreenshot | 18 |
| recording | 17 |
| settings_save | 15 |
| ai_screenshot | 16 |
| scrollshot | 14 |
| annotation | 10 |
| toolbar_shortcut | 10 |
| compatibility | 5 |
| screenshot | 9 |
| event_tracking | 2 |

**失败用例列表**:

1. yaml/ai_screenshot/test_ai_screenshot_002.yaml::AI截图功能-截图工具栏AI图标位置验证
2. yaml/ai_screenshot/test_ai_screenshot_004.yaml::AI截图功能-滚动截图工具栏AI图标位置验证
3. yaml/ai_screenshot/test_ai_screenshot_005.yaml::AI截图功能-截图工具栏AI图标hover提示验证
4. yaml/ai_screenshot/test_ai_screenshot_006.yaml::AI截图功能-滚动截图工具栏AI图标hover提示验证
5. yaml/ai_screenshot/test_ai_screenshot_007.yaml::AI截图功能-快捷键A触发AI功能验证
6. yaml/ai_screenshot/test_ai_screenshot_009.yaml::AI截图功能-选择解释/总结/翻译后OCR识别中显示验证
7. yaml/ai_screenshot/test_ai_screenshot_010.yaml::AI截图功能-滚动截取复杂长图选择解释后OCR识别显示验证
8. yaml/ai_screenshot/test_ai_screenshot_011.yaml::AI截图功能-选择解释后生成中显示验证
9. yaml/ai_screenshot/test_ai_screenshot_012.yaml::AI截图功能-含中文文字内容点击解释
10. yaml/ai_screenshot/test_ai_screenshot_013.yaml::AI截图功能-含中文文字内容点击总结
11. yaml/ai_screenshot/test_ai_screenshot_014.yaml::AI截图功能-含中文文字内容点击翻译（中译英）
12. yaml/ai_screenshot/test_ai_screenshot_015.yaml::AI截图功能-含英文文字内容点击翻译（英译中）
13. yaml/ai_screenshot/test_ai_screenshot_016.yaml::AI截图功能-空白截图内容点击解释
14. yaml/ai_screenshot/test_ai_screenshot_017.yaml::AI截图功能-空白截图内容点击总结
15. yaml/ai_screenshot/test_ai_screenshot_018.yaml::AI截图功能-空白截图内容点击翻译
16. yaml/annotation/test_annotation_001.yaml::全局验证-文案检查-中文环境AI工具栏文案
17. yaml/annotation/test_annotation_005.yaml::几何工具-切换图形并修改参数再切换图形
18. yaml/annotation/test_annotation_007.yaml::几何工具-非首次使用时默认沿用最后一次使用时的选择
19. yaml/annotation/test_annotation_008.yaml::几何工具-默认选中矩形
20. yaml/annotation/test_annotation_009.yaml::几何工具-tooltips检查
21. yaml/annotation/test_annotation_011.yaml::工具栏-位置-屏幕边界时截图-屏幕右上角
22. yaml/annotation/test_annotation_012.yaml::工具栏-位置-屏幕右下角截图
23. yaml/annotation/test_annotation_013.yaml::工具栏-位置-屏幕左上角截图
24. yaml/annotation/test_annotation_014.yaml::工具栏-位置-全屏截取
25. yaml/annotation/test_annotation_015.yaml::工具栏-布局
26. yaml/compatibility/test_compat_001.yaml::兼容性测试-截图工具
27. yaml/compatibility/test_compat_002.yaml::兼容性测试-录屏工具
28. yaml/compatibility/test_compat_003.yaml::兼容性测试-区域选择
29. yaml/compatibility/test_compat_016.yaml::[acp2][截图]检查截图识别区域功能
30. yaml/compatibility/test_compat_018.yaml::[acp2][录屏]检查录屏工具栏功能
31. yaml/event_tracking/test_event_008.yaml::【埋点检查】贴图B8
32. yaml/event_tracking/test_event_009.yaml::【埋点检查】点击AI助手图标B9
33. yaml/ocr/test_ocr_001.yaml::[smoke]OCR-启动检查-快捷键启动
34. yaml/ocr/test_ocr_002.yaml::[smoke]OCR-入口检查-工具栏按钮
35. yaml/ocr/test_ocr_003.yaml::[core]OCR-文案检查-hover提示
36. yaml/ocr/test_ocr_004.yaml::[core]OCR-文案检查-快捷键预览
37. yaml/ocr/test_ocr_005.yaml::[core]OCR-识别状态
38. yaml/ocr/test_ocr_006.yaml::[smoke]OCR-复制按钮-复制部分文本
39. yaml/ocr/test_ocr_007.yaml::[smoke]OCR-保存为TXT按钮-保存文本名称
40. yaml/ocr/test_ocr_008.yaml::OCR-保存为TXT按钮-识别后修改文件名称再保存
41. yaml/ocr/test_ocr_010.yaml::[core]OCR-文本区域-复制/剪贴/全选快捷键
42. yaml/ocr/test_ocr_011.yaml::OCR-文本区域-编辑
43. yaml/ocr/test_ocr_012.yaml::[core]OCR-图像预览区域-鼠标缩放
44. yaml/ocr/test_ocr_013.yaml::[core]OCR-图像预览区域-鼠标拖拽
45. yaml/ocr/test_ocr_015.yaml::[core]OCR-多窗口支持-多窗口展示
46. yaml/ocr/test_ocr_016.yaml::OCR-主界面-界面展示
47. yaml/ocr/test_ocr_017.yaml::OCR-交互操作-识别向上滚动截取的长图
48. yaml/ocr/test_ocr_018.yaml::[core]OCR-交互操作-多次启动OCR识别
49. yaml/ocr/test_ocr_022.yaml::OCR-交互操作-模糊图像识别
50. yaml/ocr/test_ocr_023.yaml::OCR-交互操作-超过1000字的文字识别
51. yaml/ocr/test_ocr_025.yaml::OCR-交互操作-不支持的语言识别
52. yaml/ocr/test_ocr_026.yaml::OCR-交互操作-复杂背景图像文字识别
53. yaml/ocr/test_ocr_028.yaml::贴图功能栏-保存到指定位置后点击OCR识别
54. yaml/pinscreenshot/test_pinscreenshot_001.yaml::贴图工具条-多贴图场景下悬停单个贴图仅对应工具条浮现
55. yaml/pinscreenshot/test_pinscreenshot_002.yaml::贴图工具条-鼠标快速悬停后离开工具条不闪烁
56. yaml/pinscreenshot/test_pinscreenshot_003.yaml::贴图工具条-鼠标悬停贴图边缘区域工具条浮现
57. yaml/pinscreenshot/test_pinscreenshot_004.yaml::贴图工具条-鼠标悬停贴图区域工具条自动浮现
58. yaml/pinscreenshot/test_pinscreenshot_005.yaml::贴图工具条-多贴图场景下默认隐藏验证
59. yaml/pinscreenshot/test_pinscreenshot_006.yaml::贴图工具条-鼠标离开贴图区域后自动隐藏验证
60. yaml/pinscreenshot/test_pinscreenshot_007.yaml::贴图工具条-无操作时默认隐藏验证
61. yaml/pinscreenshot/test_pinscreenshot_011.yaml::截图工具栏贴图入口位置
62. yaml/pinscreenshot/test_pinscreenshot_012.yaml::贴图大小-通过鼠标拖动浮窗边角可来回放大缩小浮窗，画面流畅
63. yaml/pinscreenshot/test_pinscreenshot_013.yaml::贴图大小-通过鼠标滚轮放大缩小贴图时，以鼠标指针原点位置放大缩小
64. yaml/pinscreenshot/test_pinscreenshot_014.yaml::贴图层级检查-有多张未激活的贴图时，最新的贴图在最上层
65. yaml/pinscreenshot/test_pinscreenshot_018.yaml::贴图功能栏-使用功能栏【X】按钮，关闭贴图，不将贴图保存到剪切板
66. yaml/pinscreenshot/test_pinscreenshot_019.yaml::贴图功能栏-使用快捷键Esc，关闭贴图，不将贴图保存到剪切板
67. yaml/pinscreenshot/test_pinscreenshot_020.yaml::贴图功能栏-快捷键打开、收起功能栏选项列表
68. yaml/pinscreenshot/test_pinscreenshot_021.yaml::贴图功能栏-功能栏选项列表快捷键选择值
69. yaml/pinscreenshot/test_pinscreenshot_022.yaml::贴图功能栏-保存贴图到文件夹后通知栏消息验证
70. yaml/pinscreenshot/test_pinscreenshot_023.yaml::贴图功能栏-保存贴图后点击通知【查看】按钮，跳转到贴图保存位置
71. yaml/pinscreenshot/test_pinscreenshot_024.yaml::贴图右键菜单-右键菜单保存功能验证
72. yaml/pinscreenshot/test_pinscreenshot_025.yaml::贴图功能栏-保存贴图到剪切板后通知栏消息验证
73. yaml/pinscreenshot/test_pinscreenshot_026.yaml::快捷键面板新增贴图快捷键
74. yaml/recording/test_recording_014.yaml::异常场景-当点击结束录屏时，在快速按esc,应用程序应正常结束
75. yaml/recording/test_recording_021.yaml::录屏保存时查看通知中心内容
76. yaml/recording/test_recording_022.yaml::工具栏-位置-屏幕边界时录屏-屏幕左下角
77. yaml/recording/test_recording_023.yaml::工具栏-位置-屏幕边界时录屏-全屏截取
78. yaml/recording/test_recording_024.yaml::声音选项-有声音输入输出时选项
79. yaml/recording/test_recording_025.yaml::声音选项-快捷键选择工具
80. yaml/recording/test_recording_026.yaml::显示按键选项-快捷键选择工具
81. yaml/recording/test_recording_027.yaml::显示按键选项-鼠标取消选择工具
82. yaml/recording/test_recording_028.yaml::显示按键选项-文案提示-显示按键
83. yaml/recording/test_recording_029.yaml::显示按键选项-显示按键时倒计时页面交互
84. yaml/recording/test_recording_034.yaml::录制鼠标选项-图标交互-勾选显示光标和点击
85. yaml/recording/test_recording_036.yaml::启动录屏功能
86. yaml/recording/test_recording_049.yaml::[core]录屏时间显示-任务栏右键菜单
87. yaml/recording/test_recording_052.yaml::[core]录屏时间显示
88. yaml/screenshot/test_screenshot_003.yaml::延时截图时查看通知中心内容
89. yaml/screenshot/test_screenshot_004.yaml::[core]连续截取不同图片粘贴正常
90. yaml/screenshot/test_screenshot_005.yaml::文字-鼠标调整光标位置
91. yaml/screenshot/test_screenshot_006.yaml::区域大小-区域尺寸
92. yaml/screenshot/test_screenshot_007.yaml::最小区域-尺寸
93. yaml/screenshot/test_screenshot_008.yaml::完成截图
94. yaml/screenshot/test_screenshot_012.yaml::一指拖拽移动和调整选择框大小
95. yaml/screenshot/test_screenshot_014.yaml::【截图】截图后剪切板验证
96. yaml/screenshot/test_screenshot_015.yaml::【截图】截图添加多行文字，文字左对齐
97. yaml/scrollshot/test_scrollshot_001.yaml::滚动截图-滚动截图后，ocr识别正常
98. yaml/scrollshot/test_scrollshot_003.yaml::截长图-工具栏-选项切换
99. yaml/scrollshot/test_scrollshot_004.yaml::截长图-自动滚动-滚动到底部停止
100. yaml/scrollshot/test_scrollshot_005.yaml::截长图-异常交互-提示
101. yaml/scrollshot/test_scrollshot_008.yaml::截长图-保存文件名称
102. yaml/scrollshot/test_scrollshot_009.yaml::截长图-自动滚动-滚动区外捕捉区内点击光标
103. yaml/scrollshot/test_scrollshot_010.yaml::截长图-图像拼接-拼接固定区域占比正常
104. yaml/scrollshot/test_scrollshot_011.yaml::截长图-控制中心-自然滚动
105. yaml/scrollshot/test_scrollshot_012.yaml::截长图-手动滚动-向上滚动到顶部
106. yaml/scrollshot/test_scrollshot_013.yaml::截长图-手动滚动-向下滚动到底部
107. yaml/scrollshot/test_scrollshot_014.yaml::截长图-滚动过快-手动向下快速滚动
108. yaml/scrollshot/test_scrollshot_015.yaml::截长图-滚动方式切换-自动切手动向下滚动
109. yaml/scrollshot/test_scrollshot_016.yaml::截长图-滚动方式切换-手动切自动
110. yaml/scrollshot/test_scrollshot_017.yaml::截长图-启动-系统快捷键启动
111. yaml/settings_save/test_settings_001.yaml::储存逻辑-设置界面ui更改验证
112. yaml/settings_save/test_settings_002.yaml::储存逻辑-工具条包含保存按钮验证
113. yaml/settings_save/test_settings_003.yaml::储存逻辑-自动生成文件名
114. yaml/settings_save/test_settings_005.yaml::储存逻辑-文管窗口修改文件格式和文件名称
115. yaml/settings_save/test_settings_010.yaml::储存逻辑-保存时更新位置，修改路径为本地目录，如音乐/视频
116. yaml/settings_save/test_settings_014.yaml::储存逻辑-已截图保存在指定位置-图片
117. yaml/settings_save/test_settings_020.yaml::储存逻辑-保存时选择位置，保存默认路径
118. yaml/settings_save/test_settings_031.yaml::储存逻辑-点击''√''按钮触发复制到粘贴板
119. yaml/settings_save/test_settings_032.yaml::储存逻辑-双击截图区域触发复制到粘贴板
120. yaml/settings_save/test_settings_033.yaml::储存逻辑-按回车触发复制到粘贴板
121. yaml/settings_save/test_settings_034.yaml::储存逻辑-tooltips检查
122. yaml/settings_save/test_settings_036.yaml::几何工具-多次切换后持续沿用记忆的选择
123. yaml/settings_save/test_settings_037.yaml::几何工具-切换图形时沿用记忆的粗细与颜色
124. yaml/settings_save/test_settings_039.yaml::保存优化-截图保存路径-非剪切板，提示查看
125. yaml/settings_save/test_settings_041.yaml::截取深色背景的图片，保存后颜色，边框正常
126. yaml/settings_save/test_settings_042.yaml::保存优化-截图完成保存剪切板，显示【保存路径】按钮
127. yaml/settings_save/test_settings_043.yaml::保存提示-忽略提示
128. yaml/settings_save/test_settings_044.yaml::选项-默认选项
129. yaml/settings_save/test_settings_045.yaml::选项-选项样式
130. yaml/settings_save/test_settings_046.yaml::选项-格式选择交互
131. yaml/settings_save/test_settings_047.yaml::选项-帧数选择交互
132. yaml/settings_save/test_settings_048.yaml::选项-记忆功能
133. yaml/settings_save/test_settings_051.yaml::主屏截图-图片保存到桌面-查看截图图片
134. yaml/toolbar_shortcut/test_toolbar_004.yaml::区域大小-快捷键调整-向上缩小
135. yaml/toolbar_shortcut/test_toolbar_005.yaml::区域大小-快捷键调整-向右缩小
136. yaml/toolbar_shortcut/test_toolbar_006.yaml::区域大小-快捷键调整-向下扩大选取高度
137. yaml/toolbar_shortcut/test_toolbar_007.yaml::区域大小-快捷键调整-向左移动
138. yaml/toolbar_shortcut/test_toolbar_008.yaml::截图终端命令-帮助信息命令
139. yaml/toolbar_shortcut/test_toolbar_009.yaml::截图终端命令-版本显示命令
140. yaml/toolbar_shortcut/test_toolbar_010.yaml::截图终端命令-延时截图命令
141. yaml/toolbar_shortcut/test_toolbar_011.yaml::截图终端命令-全屏截图命令
142. yaml/toolbar_shortcut/test_toolbar_012.yaml::截图终端命令-截图快捷键-当前窗口截图命令
143. yaml/toolbar_shortcut/test_toolbar_013.yaml::截图终端命令-无提示信息截图命令

### 3.2 unknown action: exec/for_loop/shell (14个)

**根因分析**: 用例中使用了 youqu YAML 执行器不支持的 action 类型。`exec`/`shell` 用于执行 shell 命令（创建目录、修改权限、清除配置等），`for_loop` 用于循环执行步骤。这些 action 未在 youqu 的 YAML 规范中定义。

**失败用例列表**:

| # | 用例 | 不支持的 Action | 用途 |
|---|------|-----------------|------|
| 1 | settings_save/test_settings_006 | exec | 创建只读 test 目录 |
| 2 | settings_save/test_settings_009 | exec | 创建测试文件夹 |
| 3 | settings_save/test_settings_013 | exec | 创建测试文件夹 |
| 4 | settings_save/test_settings_015 | exec | 清除配置并创建只读 test 目录 |
| 5 | settings_save/test_settings_018 | exec | 清除配置并创建测试文件夹 |
| 6 | settings_save/test_settings_019 | exec | 清除配置 |
| 7 | settings_save/test_settings_023 | exec | 清除配置并创建测试文件夹 |
| 8 | settings_save/test_settings_024 | exec | 清除配置并创建只读 test 目录 |
| 9 | settings_save/test_settings_027 | exec | 清除配置并创建测试文件夹 |
| 10 | settings_save/test_settings_028 | exec | 清除配置 |
| 11 | settings_save/test_settings_029 | exec | 清除配置 |
| 12 | settings_save/test_settings_030 | exec | 清除配置 |
| 13 | settings_save/test_settings_040 | exec | 移除图片文件夹写权限 |
| 14 | settings_save/test_settings_050 | exec | 创建测试图片 a.png |
| 15 | recording/test_recording_020 | shell | 移除 Videos 目录权限 |
| 16 | pinscreenshot/test_pinscreenshot_015 | for_loop | 循环创建 30 张贴图 |

### 3.3 ElementNotFound: 未找到UI元素 (6个)

**根因分析**: AT-SPI 在录屏/截图全屏模式下无法通过 accessibility 接口定位到工具栏上的目标按钮。全屏覆盖层没有常规窗口结构，导致 dogtail 无法通过 name/role 查找到元素。

**失败用例列表**:

| # | 用例 | 未找到的元素 |
|---|------|-------------|
| 1 | recording/test_recording_001 | `$//recorder_button/` |
| 2 | recording/test_recording_002 | `$//record_option_but/` |
| 3 | recording/test_recording_004 | `$//DTitlebarDWindowOptionButton/` |
| 4 | recording/test_recording_005 | `$//DTitlebarDWindowOptionButton/` |
| 5 | recording/test_recording_013 | `$//desktopAction/` |
| 6 | screenshot/test_screenshot_013 | `$//toolbar_close_button/` |

### 3.4 DBus 接口不存在 (2个)

**根因分析**: 用例通过 DBus 调用截图录屏服务接口，但当前系统环境中对应的 DBus 对象路径或接口不存在。

**失败用例列表**:

| # | 用例 | DBus 错误详情 |
|---|------|---------------|
| 1 | ai_assistant/test_ai_assistant_008 | `org.freedesktop.DBus.Error.UnknownInterface: No such interface 'com.deepin.Screenshot.DBusScreenshotService'` |
| 2 | event_tracking/test_event_003 | `org.freedesktop.DBus.Error.UnknownObject: No such object path '/com/deepin/Screenshot'` |

## 4. 修复建议

### 4.1 修复优先级 P0: ApplicationStartError 连锁失败

**问题**: 用例间状态依赖，一个用例退出截图后应用进程消失，后续所有用例连锁失败。

**建议**:
- 在 `conftest.py` 中添加 fixture，在每个用例执行前检查 deepin-screen-recorder 进程，如果不存在则重新启动
- 或者修改 teardown 逻辑，确保退出截图模式后应用回到待命状态而非完全退出

### 4.2 修复优先级 P1: 不支持的 Action

**问题**: 14 个用例使用了 `exec`/`shell`/`for_loop` action，youqu 不支持。

**建议**:
- 将 `exec`/`shell` 步骤改为 `dbus_call` 或在 conftest.py 中通过 fixture 预处理环境
- 将 `for_loop` 拆分为多个独立步骤，或使用 pytest 的 `parametrize` + `repeat` 实现

### 4.3 修复优先级 P1: ElementNotFound

**问题**: 6 个用例在全屏模式下找不到 UI 元素。

**建议**:
- 检查 elements.yaml 中的元素 name 是否与实际应用的 accessibility name 一致
- 考虑使用坐标点击（x/y 坐标）替代 AT-SPI 元素查找

### 4.4 修复优先级 P2: DBus 接口

**问题**: 2 个用例调用的 DBus 接口在当前环境不存在。

**建议**:
- 确认被测系统是否安装了对应版本的 deepin-screen-recorder
- 或在用例中添加 DBus 接口存在性检查，不存在时 skip

## 5. 本次前置修复记录

本次执行前对 YAML 用例进行了以下修复（共 20 个文件）：

1. **assert 格式修正**: `assert: exists` → `assert:\n  - type: element_visible`（17 个文件）
2. **selector 格式修正**: `selector: 'role: panel name: .*'` → dict 格式（11 个文件）
3. **YAML 语法修正**: description 中未转义的单引号（3 个文件）
4. **app 字段修正**: `app: ''` → `app: deepin-screen-recorder`（316 个文件）

修复后全部 316 个用例收集通过，0 collection errors。
