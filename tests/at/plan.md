# AT 录制计划: deepin-screen-recorder

生成时间: 2026-08-19T10:07:53.498865
模块数: 5
总用例数: 316

---

## 1. /V25_2500（测试部专用）/B类解耦商店应用/截图录屏(#110591)

- slug: `v25_2500_测试部专用_b类解耦商店应用_截图录屏_110591`
- 用例数: 1
- 状态: pending

### 录制指引

模块: /V25_2500（测试部专用）/B类解耦商店应用/截图录屏(#110591)
用例数: 1

操作指引 (来自帮助手册):
# 截图

在截图模式下，单击工具栏上的按钮，您可以在截图区域进行图形绘制、文字批注等操作，并可以识别图中文字、将图片置顶贴在桌面上、使用**AI截图**对截图内容进行解释、总结、翻译等。


![0|截图工具栏](fig/capturebar.png)

<table class="block1">
    <caption>工具栏说明</caption>
    <tbody>
        <tr>
            <td style="text-align:center;"><img src="../common/bar_geometric.svg" alt="几何工具" class="inline" /></td>
            <td>几何工具</td>
            <td style="text-align:center;"><img src="../common/line_mouse.svg" alt="直线" class="inline" /></td>
            <td>直线</td>
            <td style="text-align:center;"><img src="../common/Arrow_normal.svg" alt="箭头" class="inline" /></td>
            <td>箭头</td>
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/Combined_Shape.svg" alt="画笔" class="inline" /></td>
            <td>画笔</td>
            <td style="text-align:center;"><img src="../common/Mosaic_normal.svg" alt="模糊" class="inline" /></td>
            <td>模糊</td>
            <td style="text-align:center;"><img src="../common/text_normal.svg" alt="文字" class="inline" /></td>
            <td>文字</td>
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/scrollshot.svg" alt="滚动截图" class="inline" /></td>
            <td>滚动截图</td>
            <td style="text-align:center;"><img src="../common/ocr.svg" alt="识别文字" class="inline" /></td>
            <td>识别文字</td>
            <td style="text-align:center;"><img src="../common/pin.svg" alt="贴图" class="inline" /></td>
            <td>贴图</td>
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/bar_ai.svg" alt="AI截图" class="inline" /></td>
            <td>AI截图</td>
            <td style="text-align:center;"><img src="../common/bar_settings.svg" alt="设置" class="inline" /></td>
            <td>设置</td>
            <td style="text-align:center;"><img src="../common/bar_save.svg" alt="保存到本地" class="inline" /></td>
            <td>保存到本地</td>            
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/close.svg" alt="关闭" class="inline" /></td>
            <td>关闭</td>
            <td style="text-align:center;"><img src="../common/done.svg" alt="复制到剪贴板" class="inline" /></td>
            <td>复制到剪贴板</td>            
        </tr>
    </tbody>
</table>


> ![notes](../common/notes.svg) 说明：如果您已经对工具栏和属性栏中如线条粗细、字体大小等进行了选择，截图录屏会保存设置，再次启动截图录屏，将默认使用该设置，您也可以重新进行选择。

### 绘制图形

通过截图工具栏上的**几何工具**、**直线**、**箭头**、**画笔**按钮可以绘制简单的图形，并支持设置线条的粗细和颜色。

![1|绘制图形](fig/ellipsetool.png)

### 修改图形

如果您在截图中绘制了图形，可以对图形进行修改和移动。


#### 编辑图形

1. 将鼠标指针置于图形的边线上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 单击鼠标左键，进入图形的编辑模式，您可以：
   - 按下键盘上的 **Delete** 键，删除图形。
   - 将鼠标指针置于编辑框上，此时鼠标指针变成 ![双箭头](../common/Mouse_Arrow.svg)，通过拖动鼠标指针，来放大或缩小图形区域。
   - 将鼠标指针置于编辑框的 ![旋转](../common/icon_rotate.svg) 上，此时鼠标指针变成 ![旋转](../common/rotate_mouse.svg)，通过拖动鼠标指针，来旋转图形。
3. 在编辑框外单击鼠标左键，退出编辑模式。

#### 移动图形

1. 将鼠标指针置于图形的边线上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 按住鼠标左键不放，将图形移动到截图区域的任何位置。

### 模糊工具

1. 在截图工具栏中，单击 ![模糊](../common/Mosaic_normal.svg)。
2. 在工具栏展开的面板中，可以选择模糊的样式，默认为![模糊](../common/Mosaic_normal.svg)马赛克样式，亦或是![模糊](../common/vague_normal.svg)高斯模糊样式。
3. 可以选择绘制的方式，如![模糊](../common/rectangle_normal.svg)矩形框选，![模糊](../common/oval_normal.svg)椭圆形框选，![模糊](../common/brush.svg)自由涂抹。
4. 工具栏展开的面板的最右侧，可以左右拖动，选择模糊的强度。
5. 与其他图形一样，绘制的模糊图形也支持修改和移动。

### 添加文字批注

对截取的图片进行文字补充和说明，帮助他人更清晰地了解截取的图片。&nbsp;&nbsp;

1. 在截图工具栏中，单击 ![文字](../common/text_normal.svg)。
2. 在工具栏展开的面板中，选择字号和颜色，有四种颜色可供选择。
3. 将鼠标指针置于截取的图片上，此时鼠标指针变为 ![文字指针](../common/text_mouse.svg)。
4. 单击要添加批注的地方，将出现一个待输入的文本框。
5. 在文本框中输入文字。


### 修改文字批注

如果您已经添加了文字批注，可以对文字批注进行修改和移动。

#### 移动文字批注

1. 将鼠标指针置于文字上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 按住鼠标左键不放，将文字移到截图区域的任何位置。

#### 编辑文字批注

1. 将鼠标指针置于文字上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 双击鼠标左键，进入文字编辑模式，您可以：
   - 按下键盘上的 **Delete** 键，删除文字批注。
   - 在文本框内，修改文字信息。
3. 在文本框外单击鼠标左键，退出编辑模式。

### 滚动截图

滚动截图功能不仅可以截取屏幕内可见的内容，还能截取超过屏幕外的内容，满足一次性截取长图的需求。

前提条件： **控制中心 > 个性化** 中的窗口特效功能已开启。

1. 选择截图区域后，单击工具栏上的滚动截图按钮 ![icon](../common/scrollshot.svg)。
2. 请将鼠标光标保持在滚动区域内，您可以使用自动滚动或手动滚动截图，也可以交替使用两种方式：

   - **自动滚动**：在截图区域中单击鼠标左键，光标所在的截图区域自动向上滚动，此时截取的图片会拼接为一张完整的长图。自动滚动过程中，在截图区域内单击鼠标左键，或者将光标移出截图区域，暂停截图；再次单击截图区域，继续截图。

   - **手动滚动**：将鼠标光标置于截图区域中，向上或者向下滚动鼠标滚轮截取图片。停止滚动鼠标滚轮，暂停截图；再次滚动鼠标滚轮，继续截图。

3. 单击工具栏中的截图按钮 ![icon](../common/done.svg) 保存长图。

注意事项：
- 截图区域尽量不超出滚动范围，避免框选整个窗口、全屏等。
- 截图区域不宜过小，建议高度大于等于50px。
- 截图区域中尽量避免存在多个滚动区域。
- 滚动截图区域中尽量不包含大量重复内容，如空白区域、相同颜色、大段相同文字等。
- 滚动截图区域中尽量不包含动图、视频、水印、纯色背景等信息。
- 滚动操作时保持垂直方向偏移，避免滑动滚动条。
- 滚动操作时尽量不要切换窗口。
- 滚动操作速度不宜过快。


### 识别文字

对于截图区域中无法复制的内容，可以使用识别文字功能提取文字。

1. 选择截图区域后，单击工具栏上的文字识别按钮 ![ocr](../common/ocr.svg)，弹出图文识别窗口，识别图片中的文字。
2. 您可以对识别出的文字进行编辑、复制或保存为TXT等操作。

![ocr](fig/ocr.png)

### 贴图

通过贴图功能将截图变为桌面浮窗，方便您快速查阅、对比和梳理信息。

选择需要截取的区域后，单击工具栏上的贴图按钮 ![icon](../common/pin.svg)，截图浮于桌面上即为贴图，您可以对贴图进行如下操作：

- 通过鼠标移动图片的位置。
- 将鼠标指针置于图片上，向上或向下滚动鼠标滚轮等比缩放图片。
- 单击按钮 ![icon](../common/done.svg) 保存图片。

![0|贴图](fig/pin_pic.png)

### AI截图

在截图工具栏中，单击按钮<img src="../common/bar_ai.svg" alt="AI截图" class="inline" />展开AI菜单，您可以：

- 选择 **解释**，让**AI随航**为您解释截图中的文字；
- 选择 **总结**，让**AI随航**为您总结截图中的文字；
- 选择 **翻译**，让**AI随航**为您翻译截图中的文字；
- 选择 **问问AI**，直接唤起**小U助手**并将截图发送至对话框，以进行更深入的对话。

![0|AI截图](fig/ai_tools.png)

### 延时截图

您可以通过延时截图功能，捕捉特定状态或时间下的屏幕内容。

- 使用快捷键 **Ctrl** + ![icon](../common/Print.svg)，启动延时截图。
- 在终端中执行命令 “deepin-screen-recorder -d n”，其中n表示以秒为单位的延时时长。 

### 设置

您还可以在截图工具栏中，单击按钮<img src="../common/bar_settings.svg" alt="设置" class="inline" />，展开设置菜单：

![0|option](fig/screen_setting.png)

- 选择**无边框/投影/外边框/样机**，设置图片边框效果。
- 选择 **PNG/JPG/BMP**，设置图片格式。
- 选择 **显示光标**，光标在截图区域内，截图生成图片时会同时显示光标所在的位置和样式。

### 保存截图

单击工具栏中的按钮![icon](../common/bar_save.svg)或单击鼠标右键选择 **保存**，可将截图保存在本地，单击该按钮右侧的小图标![icon](../common/dropdown.svg)展开选项可设置保存规则。

- 选择 **每次询问**，可在每次点击保存按钮后选择保存路径。
- 选择 **指定位置** > **桌面**，将图片保存到桌面。
- 选择 **指定位置** > **图片**，截图文件自动保存到图片文件夹目录下。
- 选择 **指定位置** > **保存时选择位置**，可在本次点击保存按钮后选择保存路径，下次截图保存时会默认保存至该路径，除非另行设置。

### 复制到剪贴板

单击工具栏中的按钮 ![icon](../common/done.svg)，或双击鼠标左键，或使用快捷键 **Ctrl + S**，将图片保存到剪贴板中。

录制提示: 打开应用 → 按用例步骤操作 → 每个操作后等待界面稳定 → 完成后按 q 结束

---

## 2. /V25_2500（测试部专用）/B类解耦商店应用/截图录屏/AI全量(#128873)

- slug: `v25_2500_测试部专用_b类解耦商店应用_截图录屏_ai全量_128873`
- 用例数: 189
- 状态: pending

### 录制指引

模块: /V25_2500（测试部专用）/B类解耦商店应用/截图录屏/AI全量(#128873)
用例数: 189

操作指引 (来自帮助手册):
# 截图

在截图模式下，单击工具栏上的按钮，您可以在截图区域进行图形绘制、文字批注等操作，并可以识别图中文字、将图片置顶贴在桌面上、使用**AI截图**对截图内容进行解释、总结、翻译等。


![0|截图工具栏](fig/capturebar.png)

<table class="block1">
    <caption>工具栏说明</caption>
    <tbody>
        <tr>
            <td style="text-align:center;"><img src="../common/bar_geometric.svg" alt="几何工具" class="inline" /></td>
            <td>几何工具</td>
            <td style="text-align:center;"><img src="../common/line_mouse.svg" alt="直线" class="inline" /></td>
            <td>直线</td>
            <td style="text-align:center;"><img src="../common/Arrow_normal.svg" alt="箭头" class="inline" /></td>
            <td>箭头</td>
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/Combined_Shape.svg" alt="画笔" class="inline" /></td>
            <td>画笔</td>
            <td style="text-align:center;"><img src="../common/Mosaic_normal.svg" alt="模糊" class="inline" /></td>
            <td>模糊</td>
            <td style="text-align:center;"><img src="../common/text_normal.svg" alt="文字" class="inline" /></td>
            <td>文字</td>
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/scrollshot.svg" alt="滚动截图" class="inline" /></td>
            <td>滚动截图</td>
            <td style="text-align:center;"><img src="../common/ocr.svg" alt="识别文字" class="inline" /></td>
            <td>识别文字</td>
            <td style="text-align:center;"><img src="../common/pin.svg" alt="贴图" class="inline" /></td>
            <td>贴图</td>
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/bar_ai.svg" alt="AI截图" class="inline" /></td>
            <td>AI截图</td>
            <td style="text-align:center;"><img src="../common/bar_settings.svg" alt="设置" class="inline" /></td>
            <td>设置</td>
            <td style="text-align:center;"><img src="../common/bar_save.svg" alt="保存到本地" class="inline" /></td>
            <td>保存到本地</td>            
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/close.svg" alt="关闭" class="inline" /></td>
            <td>关闭</td>
            <td style="text-align:center;"><img src="../common/done.svg" alt="复制到剪贴板" class="inline" /></td>
            <td>复制到剪贴板</td>            
        </tr>
    </tbody>
</table>


> ![notes](../common/notes.svg) 说明：如果您已经对工具栏和属性栏中如线条粗细、字体大小等进行了选择，截图录屏会保存设置，再次启动截图录屏，将默认使用该设置，您也可以重新进行选择。

### 绘制图形

通过截图工具栏上的**几何工具**、**直线**、**箭头**、**画笔**按钮可以绘制简单的图形，并支持设置线条的粗细和颜色。

![1|绘制图形](fig/ellipsetool.png)

### 修改图形

如果您在截图中绘制了图形，可以对图形进行修改和移动。


#### 编辑图形

1. 将鼠标指针置于图形的边线上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 单击鼠标左键，进入图形的编辑模式，您可以：
   - 按下键盘上的 **Delete** 键，删除图形。
   - 将鼠标指针置于编辑框上，此时鼠标指针变成 ![双箭头](../common/Mouse_Arrow.svg)，通过拖动鼠标指针，来放大或缩小图形区域。
   - 将鼠标指针置于编辑框的 ![旋转](../common/icon_rotate.svg) 上，此时鼠标指针变成 ![旋转](../common/rotate_mouse.svg)，通过拖动鼠标指针，来旋转图形。
3. 在编辑框外单击鼠标左键，退出编辑模式。

#### 移动图形

1. 将鼠标指针置于图形的边线上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 按住鼠标左键不放，将图形移动到截图区域的任何位置。

### 模糊工具

1. 在截图工具栏中，单击 ![模糊](../common/Mosaic_normal.svg)。
2. 在工具栏展开的面板中，可以选择模糊的样式，默认为![模糊](../common/Mosaic_normal.svg)马赛克样式，亦或是![模糊](../common/vague_normal.svg)高斯模糊样式。
3. 可以选择绘制的方式，如![模糊](../common/rectangle_normal.svg)矩形框选，![模糊](../common/oval_normal.svg)椭圆形框选，![模糊](../common/brush.svg)自由涂抹。
4. 工具栏展开的面板的最右侧，可以左右拖动，选择模糊的强度。
5. 与其他图形一样，绘制的模糊图形也支持修改和移动。

### 添加文字批注

对截取的图片进行文字补充和说明，帮助他人更清晰地了解截取的图片。&nbsp;&nbsp;

1. 在截图工具栏中，单击 ![文字](../common/text_normal.svg)。
2. 在工具栏展开的面板中，选择字号和颜色，有四种颜色可供选择。
3. 将鼠标指针置于截取的图片上，此时鼠标指针变为 ![文字指针](../common/text_mouse.svg)。
4. 单击要添加批注的地方，将出现一个待输入的文本框。
5. 在文本框中输入文字。


### 修改文字批注

如果您已经添加了文字批注，可以对文字批注进行修改和移动。

#### 移动文字批注

1. 将鼠标指针置于文字上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 按住鼠标左键不放，将文字移到截图区域的任何位置。

#### 编辑文字批注

1. 将鼠标指针置于文字上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 双击鼠标左键，进入文字编辑模式，您可以：
   - 按下键盘上的 **Delete** 键，删除文字批注。
   - 在文本框内，修改文字信息。
3. 在文本框外单击鼠标左键，退出编辑模式。

### 滚动截图

滚动截图功能不仅可以截取屏幕内可见的内容，还能截取超过屏幕外的内容，满足一次性截取长图的需求。

前提条件： **控制中心 > 个性化** 中的窗口特效功能已开启。

1. 选择截图区域后，单击工具栏上的滚动截图按钮 ![icon](../common/scrollshot.svg)。
2. 请将鼠标光标保持在滚动区域内，您可以使用自动滚动或手动滚动截图，也可以交替使用两种方式：

   - **自动滚动**：在截图区域中单击鼠标左键，光标所在的截图区域自动向上滚动，此时截取的图片会拼接为一张完整的长图。自动滚动过程中，在截图区域内单击鼠标左键，或者将光标移出截图区域，暂停截图；再次单击截图区域，继续截图。

   - **手动滚动**：将鼠标光标置于截图区域中，向上或者向下滚动鼠标滚轮截取图片。停止滚动鼠标滚轮，暂停截图；再次滚动鼠标滚轮，继续截图。

3. 单击工具栏中的截图按钮 ![icon](../common/done.svg) 保存长图。

注意事项：
- 截图区域尽量不超出滚动范围，避免框选整个窗口、全屏等。
- 截图区域不宜过小，建议高度大于等于50px。
- 截图区域中尽量避免存在多个滚动区域。
- 滚动截图区域中尽量不包含大量重复内容，如空白区域、相同颜色、大段相同文字等。
- 滚动截图区域中尽量不包含动图、视频、水印、纯色背景等信息。
- 滚动操作时保持垂直方向偏移，避免滑动滚动条。
- 滚动操作时尽量不要切换窗口。
- 滚动操作速度不宜过快。


### 识别文字

对于截图区域中无法复制的内容，可以使用识别文字功能提取文字。

1. 选择截图区域后，单击工具栏上的文字识别按钮 ![ocr](../common/ocr.svg)，弹出图文识别窗口，识别图片中的文字。
2. 您可以对识别出的文字进行编辑、复制或保存为TXT等操作。

![ocr](fig/ocr.png)

### 贴图

通过贴图功能将截图变为桌面浮窗，方便您快速查阅、对比和梳理信息。

选择需要截取的区域后，单击工具栏上的贴图按钮 ![icon](../common/pin.svg)，截图浮于桌面上即为贴图，您可以对贴图进行如下操作：

- 通过鼠标移动图片的位置。
- 将鼠标指针置于图片上，向上或向下滚动鼠标滚轮等比缩放图片。
- 单击按钮 ![icon](../common/done.svg) 保存图片。

![0|贴图](fig/pin_pic.png)

### AI截图

在截图工具栏中，单击按钮<img src="../common/bar_ai.svg" alt="AI截图" class="inline" />展开AI菜单，您可以：

- 选择 **解释**，让**AI随航**为您解释截图中的文字；
- 选择 **总结**，让**AI随航**为您总结截图中的文字；
- 选择 **翻译**，让**AI随航**为您翻译截图中的文字；
- 选择 **问问AI**，直接唤起**小U助手**并将截图发送至对话框，以进行更深入的对话。

![0|AI截图](fig/ai_tools.png)

### 延时截图

您可以通过延时截图功能，捕捉特定状态或时间下的屏幕内容。

- 使用快捷键 **Ctrl** + ![icon](../common/Print.svg)，启动延时截图。
- 在终端中执行命令 “deepin-screen-recorder -d n”，其中n表示以秒为单位的延时时长。 

### 设置

您还可以在截图工具栏中，单击按钮<img src="../common/bar_settings.svg" alt="设置" class="inline" />，展开设置菜单：

![0|option](fig/screen_setting.png)

- 选择**无边框/投影/外边框/样机**，设置图片边框效果。
- 选择 **PNG/JPG/BMP**，设置图片格式。
- 选择 **显示光标**，光标在截图区域内，截图生成图片时会同时显示光标所在的位置和样式。

### 保存截图

单击工具栏中的按钮![icon](../common/bar_save.svg)或单击鼠标右键选择 **保存**，可将截图保存在本地，单击该按钮右侧的小图标![icon](../common/dropdown.svg)展开选项可设置保存规则。

- 选择 **每次询问**，可在每次点击保存按钮后选择保存路径。
- 选择 **指定位置** > **桌面**，将图片保存到桌面。
- 选择 **指定位置** > **图片**，截图文件自动保存到图片文件夹目录下。
- 选择 **指定位置** > **保存时选择位置**，可在本次点击保存按钮后选择保存路径，下次截图保存时会默认保存至该路径，除非另行设置。

### 复制到剪贴板

单击工具栏中的按钮 ![icon](../common/done.svg)，或双击鼠标左键，或使用快捷键 **Ctrl + S**，将图片保存到剪贴板中。

录制提示: 打开应用 → 按用例步骤操作 → 每个操作后等待界面稳定 → 完成后按 q 结束

---

## 3. /V25_2500（测试部专用）/B类解耦商店应用/截图录屏/1070-新需求(#115917)

- slug: `v25_2500_测试部专用_b类解耦商店应用_截图录屏_1070-新需求_115917`
- 用例数: 9
- 状态: pending

### 录制指引

模块: /V25_2500（测试部专用）/B类解耦商店应用/截图录屏/1070-新需求(#115917)
用例数: 9

操作指引 (来自帮助手册):
# 截图

在截图模式下，单击工具栏上的按钮，您可以在截图区域进行图形绘制、文字批注等操作，并可以识别图中文字、将图片置顶贴在桌面上、使用**AI截图**对截图内容进行解释、总结、翻译等。


![0|截图工具栏](fig/capturebar.png)

<table class="block1">
    <caption>工具栏说明</caption>
    <tbody>
        <tr>
            <td style="text-align:center;"><img src="../common/bar_geometric.svg" alt="几何工具" class="inline" /></td>
            <td>几何工具</td>
            <td style="text-align:center;"><img src="../common/line_mouse.svg" alt="直线" class="inline" /></td>
            <td>直线</td>
            <td style="text-align:center;"><img src="../common/Arrow_normal.svg" alt="箭头" class="inline" /></td>
            <td>箭头</td>
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/Combined_Shape.svg" alt="画笔" class="inline" /></td>
            <td>画笔</td>
            <td style="text-align:center;"><img src="../common/Mosaic_normal.svg" alt="模糊" class="inline" /></td>
            <td>模糊</td>
            <td style="text-align:center;"><img src="../common/text_normal.svg" alt="文字" class="inline" /></td>
            <td>文字</td>
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/scrollshot.svg" alt="滚动截图" class="inline" /></td>
            <td>滚动截图</td>
            <td style="text-align:center;"><img src="../common/ocr.svg" alt="识别文字" class="inline" /></td>
            <td>识别文字</td>
            <td style="text-align:center;"><img src="../common/pin.svg" alt="贴图" class="inline" /></td>
            <td>贴图</td>
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/bar_ai.svg" alt="AI截图" class="inline" /></td>
            <td>AI截图</td>
            <td style="text-align:center;"><img src="../common/bar_settings.svg" alt="设置" class="inline" /></td>
            <td>设置</td>
            <td style="text-align:center;"><img src="../common/bar_save.svg" alt="保存到本地" class="inline" /></td>
            <td>保存到本地</td>            
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/close.svg" alt="关闭" class="inline" /></td>
            <td>关闭</td>
            <td style="text-align:center;"><img src="../common/done.svg" alt="复制到剪贴板" class="inline" /></td>
            <td>复制到剪贴板</td>            
        </tr>
    </tbody>
</table>


> ![notes](../common/notes.svg) 说明：如果您已经对工具栏和属性栏中如线条粗细、字体大小等进行了选择，截图录屏会保存设置，再次启动截图录屏，将默认使用该设置，您也可以重新进行选择。

### 绘制图形

通过截图工具栏上的**几何工具**、**直线**、**箭头**、**画笔**按钮可以绘制简单的图形，并支持设置线条的粗细和颜色。

![1|绘制图形](fig/ellipsetool.png)

### 修改图形

如果您在截图中绘制了图形，可以对图形进行修改和移动。


#### 编辑图形

1. 将鼠标指针置于图形的边线上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 单击鼠标左键，进入图形的编辑模式，您可以：
   - 按下键盘上的 **Delete** 键，删除图形。
   - 将鼠标指针置于编辑框上，此时鼠标指针变成 ![双箭头](../common/Mouse_Arrow.svg)，通过拖动鼠标指针，来放大或缩小图形区域。
   - 将鼠标指针置于编辑框的 ![旋转](../common/icon_rotate.svg) 上，此时鼠标指针变成 ![旋转](../common/rotate_mouse.svg)，通过拖动鼠标指针，来旋转图形。
3. 在编辑框外单击鼠标左键，退出编辑模式。

#### 移动图形

1. 将鼠标指针置于图形的边线上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 按住鼠标左键不放，将图形移动到截图区域的任何位置。

### 模糊工具

1. 在截图工具栏中，单击 ![模糊](../common/Mosaic_normal.svg)。
2. 在工具栏展开的面板中，可以选择模糊的样式，默认为![模糊](../common/Mosaic_normal.svg)马赛克样式，亦或是![模糊](../common/vague_normal.svg)高斯模糊样式。
3. 可以选择绘制的方式，如![模糊](../common/rectangle_normal.svg)矩形框选，![模糊](../common/oval_normal.svg)椭圆形框选，![模糊](../common/brush.svg)自由涂抹。
4. 工具栏展开的面板的最右侧，可以左右拖动，选择模糊的强度。
5. 与其他图形一样，绘制的模糊图形也支持修改和移动。

### 添加文字批注

对截取的图片进行文字补充和说明，帮助他人更清晰地了解截取的图片。&nbsp;&nbsp;

1. 在截图工具栏中，单击 ![文字](../common/text_normal.svg)。
2. 在工具栏展开的面板中，选择字号和颜色，有四种颜色可供选择。
3. 将鼠标指针置于截取的图片上，此时鼠标指针变为 ![文字指针](../common/text_mouse.svg)。
4. 单击要添加批注的地方，将出现一个待输入的文本框。
5. 在文本框中输入文字。


### 修改文字批注

如果您已经添加了文字批注，可以对文字批注进行修改和移动。

#### 移动文字批注

1. 将鼠标指针置于文字上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 按住鼠标左键不放，将文字移到截图区域的任何位置。

#### 编辑文字批注

1. 将鼠标指针置于文字上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 双击鼠标左键，进入文字编辑模式，您可以：
   - 按下键盘上的 **Delete** 键，删除文字批注。
   - 在文本框内，修改文字信息。
3. 在文本框外单击鼠标左键，退出编辑模式。

### 滚动截图

滚动截图功能不仅可以截取屏幕内可见的内容，还能截取超过屏幕外的内容，满足一次性截取长图的需求。

前提条件： **控制中心 > 个性化** 中的窗口特效功能已开启。

1. 选择截图区域后，单击工具栏上的滚动截图按钮 ![icon](../common/scrollshot.svg)。
2. 请将鼠标光标保持在滚动区域内，您可以使用自动滚动或手动滚动截图，也可以交替使用两种方式：

   - **自动滚动**：在截图区域中单击鼠标左键，光标所在的截图区域自动向上滚动，此时截取的图片会拼接为一张完整的长图。自动滚动过程中，在截图区域内单击鼠标左键，或者将光标移出截图区域，暂停截图；再次单击截图区域，继续截图。

   - **手动滚动**：将鼠标光标置于截图区域中，向上或者向下滚动鼠标滚轮截取图片。停止滚动鼠标滚轮，暂停截图；再次滚动鼠标滚轮，继续截图。

3. 单击工具栏中的截图按钮 ![icon](../common/done.svg) 保存长图。

注意事项：
- 截图区域尽量不超出滚动范围，避免框选整个窗口、全屏等。
- 截图区域不宜过小，建议高度大于等于50px。
- 截图区域中尽量避免存在多个滚动区域。
- 滚动截图区域中尽量不包含大量重复内容，如空白区域、相同颜色、大段相同文字等。
- 滚动截图区域中尽量不包含动图、视频、水印、纯色背景等信息。
- 滚动操作时保持垂直方向偏移，避免滑动滚动条。
- 滚动操作时尽量不要切换窗口。
- 滚动操作速度不宜过快。


### 识别文字

对于截图区域中无法复制的内容，可以使用识别文字功能提取文字。

1. 选择截图区域后，单击工具栏上的文字识别按钮 ![ocr](../common/ocr.svg)，弹出图文识别窗口，识别图片中的文字。
2. 您可以对识别出的文字进行编辑、复制或保存为TXT等操作。

![ocr](fig/ocr.png)

### 贴图

通过贴图功能将截图变为桌面浮窗，方便您快速查阅、对比和梳理信息。

选择需要截取的区域后，单击工具栏上的贴图按钮 ![icon](../common/pin.svg)，截图浮于桌面上即为贴图，您可以对贴图进行如下操作：

- 通过鼠标移动图片的位置。
- 将鼠标指针置于图片上，向上或向下滚动鼠标滚轮等比缩放图片。
- 单击按钮 ![icon](../common/done.svg) 保存图片。

![0|贴图](fig/pin_pic.png)

### AI截图

在截图工具栏中，单击按钮<img src="../common/bar_ai.svg" alt="AI截图" class="inline" />展开AI菜单，您可以：

- 选择 **解释**，让**AI随航**为您解释截图中的文字；
- 选择 **总结**，让**AI随航**为您总结截图中的文字；
- 选择 **翻译**，让**AI随航**为您翻译截图中的文字；
- 选择 **问问AI**，直接唤起**小U助手**并将截图发送至对话框，以进行更深入的对话。

![0|AI截图](fig/ai_tools.png)

### 延时截图

您可以通过延时截图功能，捕捉特定状态或时间下的屏幕内容。

- 使用快捷键 **Ctrl** + ![icon](../common/Print.svg)，启动延时截图。
- 在终端中执行命令 “deepin-screen-recorder -d n”，其中n表示以秒为单位的延时时长。 

### 设置

您还可以在截图工具栏中，单击按钮<img src="../common/bar_settings.svg" alt="设置" class="inline" />，展开设置菜单：

![0|option](fig/screen_setting.png)

- 选择**无边框/投影/外边框/样机**，设置图片边框效果。
- 选择 **PNG/JPG/BMP**，设置图片格式。
- 选择 **显示光标**，光标在截图区域内，截图生成图片时会同时显示光标所在的位置和样式。

### 保存截图

单击工具栏中的按钮![icon](../common/bar_save.svg)或单击鼠标右键选择 **保存**，可将截图保存在本地，单击该按钮右侧的小图标![icon](../common/dropdown.svg)展开选项可设置保存规则。

- 选择 **每次询问**，可在每次点击保存按钮后选择保存路径。
- 选择 **指定位置** > **桌面**，将图片保存到桌面。
- 选择 **指定位置** > **图片**，截图文件自动保存到图片文件夹目录下。
- 选择 **指定位置** > **保存时选择位置**，可在本次点击保存按钮后选择保存路径，下次截图保存时会默认保存至该路径，除非另行设置。

### 复制到剪贴板

单击工具栏中的按钮 ![icon](../common/done.svg)，或双击鼠标左键，或使用快捷键 **Ctrl + S**，将图片保存到剪贴板中。

录制提示: 打开应用 → 按用例步骤操作 → 每个操作后等待界面稳定 → 完成后按 q 结束

---

## 4. /V25_2500（测试部专用）/B类解耦商店应用/截图录屏/1060（基线）/截图/截图区域选择(#115943)

- slug: `v25_2500_测试部专用_b类解耦商店应用_截图录屏_1060_基线_截图_截图区域选择_115943`
- 用例数: 1
- 状态: pending

### 录制指引

模块: /V25_2500（测试部专用）/B类解耦商店应用/截图录屏/1060（基线）/截图/截图区域选择(#115943)
用例数: 1

操作指引 (来自帮助手册):
# 截图

在截图模式下，单击工具栏上的按钮，您可以在截图区域进行图形绘制、文字批注等操作，并可以识别图中文字、将图片置顶贴在桌面上、使用**AI截图**对截图内容进行解释、总结、翻译等。


![0|截图工具栏](fig/capturebar.png)

<table class="block1">
    <caption>工具栏说明</caption>
    <tbody>
        <tr>
            <td style="text-align:center;"><img src="../common/bar_geometric.svg" alt="几何工具" class="inline" /></td>
            <td>几何工具</td>
            <td style="text-align:center;"><img src="../common/line_mouse.svg" alt="直线" class="inline" /></td>
            <td>直线</td>
            <td style="text-align:center;"><img src="../common/Arrow_normal.svg" alt="箭头" class="inline" /></td>
            <td>箭头</td>
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/Combined_Shape.svg" alt="画笔" class="inline" /></td>
            <td>画笔</td>
            <td style="text-align:center;"><img src="../common/Mosaic_normal.svg" alt="模糊" class="inline" /></td>
            <td>模糊</td>
            <td style="text-align:center;"><img src="../common/text_normal.svg" alt="文字" class="inline" /></td>
            <td>文字</td>
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/scrollshot.svg" alt="滚动截图" class="inline" /></td>
            <td>滚动截图</td>
            <td style="text-align:center;"><img src="../common/ocr.svg" alt="识别文字" class="inline" /></td>
            <td>识别文字</td>
            <td style="text-align:center;"><img src="../common/pin.svg" alt="贴图" class="inline" /></td>
            <td>贴图</td>
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/bar_ai.svg" alt="AI截图" class="inline" /></td>
            <td>AI截图</td>
            <td style="text-align:center;"><img src="../common/bar_settings.svg" alt="设置" class="inline" /></td>
            <td>设置</td>
            <td style="text-align:center;"><img src="../common/bar_save.svg" alt="保存到本地" class="inline" /></td>
            <td>保存到本地</td>            
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/close.svg" alt="关闭" class="inline" /></td>
            <td>关闭</td>
            <td style="text-align:center;"><img src="../common/done.svg" alt="复制到剪贴板" class="inline" /></td>
            <td>复制到剪贴板</td>            
        </tr>
    </tbody>
</table>


> ![notes](../common/notes.svg) 说明：如果您已经对工具栏和属性栏中如线条粗细、字体大小等进行了选择，截图录屏会保存设置，再次启动截图录屏，将默认使用该设置，您也可以重新进行选择。

### 绘制图形

通过截图工具栏上的**几何工具**、**直线**、**箭头**、**画笔**按钮可以绘制简单的图形，并支持设置线条的粗细和颜色。

![1|绘制图形](fig/ellipsetool.png)

### 修改图形

如果您在截图中绘制了图形，可以对图形进行修改和移动。


#### 编辑图形

1. 将鼠标指针置于图形的边线上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 单击鼠标左键，进入图形的编辑模式，您可以：
   - 按下键盘上的 **Delete** 键，删除图形。
   - 将鼠标指针置于编辑框上，此时鼠标指针变成 ![双箭头](../common/Mouse_Arrow.svg)，通过拖动鼠标指针，来放大或缩小图形区域。
   - 将鼠标指针置于编辑框的 ![旋转](../common/icon_rotate.svg) 上，此时鼠标指针变成 ![旋转](../common/rotate_mouse.svg)，通过拖动鼠标指针，来旋转图形。
3. 在编辑框外单击鼠标左键，退出编辑模式。

#### 移动图形

1. 将鼠标指针置于图形的边线上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 按住鼠标左键不放，将图形移动到截图区域的任何位置。

### 模糊工具

1. 在截图工具栏中，单击 ![模糊](../common/Mosaic_normal.svg)。
2. 在工具栏展开的面板中，可以选择模糊的样式，默认为![模糊](../common/Mosaic_normal.svg)马赛克样式，亦或是![模糊](../common/vague_normal.svg)高斯模糊样式。
3. 可以选择绘制的方式，如![模糊](../common/rectangle_normal.svg)矩形框选，![模糊](../common/oval_normal.svg)椭圆形框选，![模糊](../common/brush.svg)自由涂抹。
4. 工具栏展开的面板的最右侧，可以左右拖动，选择模糊的强度。
5. 与其他图形一样，绘制的模糊图形也支持修改和移动。

### 添加文字批注

对截取的图片进行文字补充和说明，帮助他人更清晰地了解截取的图片。&nbsp;&nbsp;

1. 在截图工具栏中，单击 ![文字](../common/text_normal.svg)。
2. 在工具栏展开的面板中，选择字号和颜色，有四种颜色可供选择。
3. 将鼠标指针置于截取的图片上，此时鼠标指针变为 ![文字指针](../common/text_mouse.svg)。
4. 单击要添加批注的地方，将出现一个待输入的文本框。
5. 在文本框中输入文字。


### 修改文字批注

如果您已经添加了文字批注，可以对文字批注进行修改和移动。

#### 移动文字批注

1. 将鼠标指针置于文字上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 按住鼠标左键不放，将文字移到截图区域的任何位置。

#### 编辑文字批注

1. 将鼠标指针置于文字上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 双击鼠标左键，进入文字编辑模式，您可以：
   - 按下键盘上的 **Delete** 键，删除文字批注。
   - 在文本框内，修改文字信息。
3. 在文本框外单击鼠标左键，退出编辑模式。

### 滚动截图

滚动截图功能不仅可以截取屏幕内可见的内容，还能截取超过屏幕外的内容，满足一次性截取长图的需求。

前提条件： **控制中心 > 个性化** 中的窗口特效功能已开启。

1. 选择截图区域后，单击工具栏上的滚动截图按钮 ![icon](../common/scrollshot.svg)。
2. 请将鼠标光标保持在滚动区域内，您可以使用自动滚动或手动滚动截图，也可以交替使用两种方式：

   - **自动滚动**：在截图区域中单击鼠标左键，光标所在的截图区域自动向上滚动，此时截取的图片会拼接为一张完整的长图。自动滚动过程中，在截图区域内单击鼠标左键，或者将光标移出截图区域，暂停截图；再次单击截图区域，继续截图。

   - **手动滚动**：将鼠标光标置于截图区域中，向上或者向下滚动鼠标滚轮截取图片。停止滚动鼠标滚轮，暂停截图；再次滚动鼠标滚轮，继续截图。

3. 单击工具栏中的截图按钮 ![icon](../common/done.svg) 保存长图。

注意事项：
- 截图区域尽量不超出滚动范围，避免框选整个窗口、全屏等。
- 截图区域不宜过小，建议高度大于等于50px。
- 截图区域中尽量避免存在多个滚动区域。
- 滚动截图区域中尽量不包含大量重复内容，如空白区域、相同颜色、大段相同文字等。
- 滚动截图区域中尽量不包含动图、视频、水印、纯色背景等信息。
- 滚动操作时保持垂直方向偏移，避免滑动滚动条。
- 滚动操作时尽量不要切换窗口。
- 滚动操作速度不宜过快。


### 识别文字

对于截图区域中无法复制的内容，可以使用识别文字功能提取文字。

1. 选择截图区域后，单击工具栏上的文字识别按钮 ![ocr](../common/ocr.svg)，弹出图文识别窗口，识别图片中的文字。
2. 您可以对识别出的文字进行编辑、复制或保存为TXT等操作。

![ocr](fig/ocr.png)

### 贴图

通过贴图功能将截图变为桌面浮窗，方便您快速查阅、对比和梳理信息。

选择需要截取的区域后，单击工具栏上的贴图按钮 ![icon](../common/pin.svg)，截图浮于桌面上即为贴图，您可以对贴图进行如下操作：

- 通过鼠标移动图片的位置。
- 将鼠标指针置于图片上，向上或向下滚动鼠标滚轮等比缩放图片。
- 单击按钮 ![icon](../common/done.svg) 保存图片。

![0|贴图](fig/pin_pic.png)

### AI截图

在截图工具栏中，单击按钮<img src="../common/bar_ai.svg" alt="AI截图" class="inline" />展开AI菜单，您可以：

- 选择 **解释**，让**AI随航**为您解释截图中的文字；
- 选择 **总结**，让**AI随航**为您总结截图中的文字；
- 选择 **翻译**，让**AI随航**为您翻译截图中的文字；
- 选择 **问问AI**，直接唤起**小U助手**并将截图发送至对话框，以进行更深入的对话。

![0|AI截图](fig/ai_tools.png)

### 延时截图

您可以通过延时截图功能，捕捉特定状态或时间下的屏幕内容。

- 使用快捷键 **Ctrl** + ![icon](../common/Print.svg)，启动延时截图。
- 在终端中执行命令 “deepin-screen-recorder -d n”，其中n表示以秒为单位的延时时长。 

### 设置

您还可以在截图工具栏中，单击按钮<img src="../common/bar_settings.svg" alt="设置" class="inline" />，展开设置菜单：

![0|option](fig/screen_setting.png)

- 选择**无边框/投影/外边框/样机**，设置图片边框效果。
- 选择 **PNG/JPG/BMP**，设置图片格式。
- 选择 **显示光标**，光标在截图区域内，截图生成图片时会同时显示光标所在的位置和样式。

### 保存截图

单击工具栏中的按钮![icon](../common/bar_save.svg)或单击鼠标右键选择 **保存**，可将截图保存在本地，单击该按钮右侧的小图标![icon](../common/dropdown.svg)展开选项可设置保存规则。

- 选择 **每次询问**，可在每次点击保存按钮后选择保存路径。
- 选择 **指定位置** > **桌面**，将图片保存到桌面。
- 选择 **指定位置** > **图片**，截图文件自动保存到图片文件夹目录下。
- 选择 **指定位置** > **保存时选择位置**，可在本次点击保存按钮后选择保存路径，下次截图保存时会默认保存至该路径，除非另行设置。

### 复制到剪贴板

单击工具栏中的按钮 ![icon](../common/done.svg)，或双击鼠标左键，或使用快捷键 **Ctrl + S**，将图片保存到剪贴板中。

录制提示: 打开应用 → 按用例步骤操作 → 每个操作后等待界面稳定 → 完成后按 q 结束

---

## 5. /V25_2500（测试部专用）/B类解耦商店应用/截图录屏/V25新需求(#129855)

- slug: `v25_2500_测试部专用_b类解耦商店应用_截图录屏_v25新需求_129855`
- 用例数: 116
- 状态: pending

### 录制指引

模块: /V25_2500（测试部专用）/B类解耦商店应用/截图录屏/V25新需求(#129855)
用例数: 116

操作指引 (来自帮助手册):
# 截图

在截图模式下，单击工具栏上的按钮，您可以在截图区域进行图形绘制、文字批注等操作，并可以识别图中文字、将图片置顶贴在桌面上、使用**AI截图**对截图内容进行解释、总结、翻译等。


![0|截图工具栏](fig/capturebar.png)

<table class="block1">
    <caption>工具栏说明</caption>
    <tbody>
        <tr>
            <td style="text-align:center;"><img src="../common/bar_geometric.svg" alt="几何工具" class="inline" /></td>
            <td>几何工具</td>
            <td style="text-align:center;"><img src="../common/line_mouse.svg" alt="直线" class="inline" /></td>
            <td>直线</td>
            <td style="text-align:center;"><img src="../common/Arrow_normal.svg" alt="箭头" class="inline" /></td>
            <td>箭头</td>
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/Combined_Shape.svg" alt="画笔" class="inline" /></td>
            <td>画笔</td>
            <td style="text-align:center;"><img src="../common/Mosaic_normal.svg" alt="模糊" class="inline" /></td>
            <td>模糊</td>
            <td style="text-align:center;"><img src="../common/text_normal.svg" alt="文字" class="inline" /></td>
            <td>文字</td>
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/scrollshot.svg" alt="滚动截图" class="inline" /></td>
            <td>滚动截图</td>
            <td style="text-align:center;"><img src="../common/ocr.svg" alt="识别文字" class="inline" /></td>
            <td>识别文字</td>
            <td style="text-align:center;"><img src="../common/pin.svg" alt="贴图" class="inline" /></td>
            <td>贴图</td>
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/bar_ai.svg" alt="AI截图" class="inline" /></td>
            <td>AI截图</td>
            <td style="text-align:center;"><img src="../common/bar_settings.svg" alt="设置" class="inline" /></td>
            <td>设置</td>
            <td style="text-align:center;"><img src="../common/bar_save.svg" alt="保存到本地" class="inline" /></td>
            <td>保存到本地</td>            
        </tr>
        <tr>
            <td style="text-align:center;"><img src="../common/close.svg" alt="关闭" class="inline" /></td>
            <td>关闭</td>
            <td style="text-align:center;"><img src="../common/done.svg" alt="复制到剪贴板" class="inline" /></td>
            <td>复制到剪贴板</td>            
        </tr>
    </tbody>
</table>


> ![notes](../common/notes.svg) 说明：如果您已经对工具栏和属性栏中如线条粗细、字体大小等进行了选择，截图录屏会保存设置，再次启动截图录屏，将默认使用该设置，您也可以重新进行选择。

### 绘制图形

通过截图工具栏上的**几何工具**、**直线**、**箭头**、**画笔**按钮可以绘制简单的图形，并支持设置线条的粗细和颜色。

![1|绘制图形](fig/ellipsetool.png)

### 修改图形

如果您在截图中绘制了图形，可以对图形进行修改和移动。


#### 编辑图形

1. 将鼠标指针置于图形的边线上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 单击鼠标左键，进入图形的编辑模式，您可以：
   - 按下键盘上的 **Delete** 键，删除图形。
   - 将鼠标指针置于编辑框上，此时鼠标指针变成 ![双箭头](../common/Mouse_Arrow.svg)，通过拖动鼠标指针，来放大或缩小图形区域。
   - 将鼠标指针置于编辑框的 ![旋转](../common/icon_rotate.svg) 上，此时鼠标指针变成 ![旋转](../common/rotate_mouse.svg)，通过拖动鼠标指针，来旋转图形。
3. 在编辑框外单击鼠标左键，退出编辑模式。

#### 移动图形

1. 将鼠标指针置于图形的边线上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 按住鼠标左键不放，将图形移动到截图区域的任何位置。

### 模糊工具

1. 在截图工具栏中，单击 ![模糊](../common/Mosaic_normal.svg)。
2. 在工具栏展开的面板中，可以选择模糊的样式，默认为![模糊](../common/Mosaic_normal.svg)马赛克样式，亦或是![模糊](../common/vague_normal.svg)高斯模糊样式。
3. 可以选择绘制的方式，如![模糊](../common/rectangle_normal.svg)矩形框选，![模糊](../common/oval_normal.svg)椭圆形框选，![模糊](../common/brush.svg)自由涂抹。
4. 工具栏展开的面板的最右侧，可以左右拖动，选择模糊的强度。
5. 与其他图形一样，绘制的模糊图形也支持修改和移动。

### 添加文字批注

对截取的图片进行文字补充和说明，帮助他人更清晰地了解截取的图片。&nbsp;&nbsp;

1. 在截图工具栏中，单击 ![文字](../common/text_normal.svg)。
2. 在工具栏展开的面板中，选择字号和颜色，有四种颜色可供选择。
3. 将鼠标指针置于截取的图片上，此时鼠标指针变为 ![文字指针](../common/text_mouse.svg)。
4. 单击要添加批注的地方，将出现一个待输入的文本框。
5. 在文本框中输入文字。


### 修改文字批注

如果您已经添加了文字批注，可以对文字批注进行修改和移动。

#### 移动文字批注

1. 将鼠标指针置于文字上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 按住鼠标左键不放，将文字移到截图区域的任何位置。

#### 编辑文字批注

1. 将鼠标指针置于文字上，此时鼠标指针变为 ![手指](fig/fingers.png)。
2. 双击鼠标左键，进入文字编辑模式，您可以：
   - 按下键盘上的 **Delete** 键，删除文字批注。
   - 在文本框内，修改文字信息。
3. 在文本框外单击鼠标左键，退出编辑模式。

### 滚动截图

滚动截图功能不仅可以截取屏幕内可见的内容，还能截取超过屏幕外的内容，满足一次性截取长图的需求。

前提条件： **控制中心 > 个性化** 中的窗口特效功能已开启。

1. 选择截图区域后，单击工具栏上的滚动截图按钮 ![icon](../common/scrollshot.svg)。
2. 请将鼠标光标保持在滚动区域内，您可以使用自动滚动或手动滚动截图，也可以交替使用两种方式：

   - **自动滚动**：在截图区域中单击鼠标左键，光标所在的截图区域自动向上滚动，此时截取的图片会拼接为一张完整的长图。自动滚动过程中，在截图区域内单击鼠标左键，或者将光标移出截图区域，暂停截图；再次单击截图区域，继续截图。

   - **手动滚动**：将鼠标光标置于截图区域中，向上或者向下滚动鼠标滚轮截取图片。停止滚动鼠标滚轮，暂停截图；再次滚动鼠标滚轮，继续截图。

3. 单击工具栏中的截图按钮 ![icon](../common/done.svg) 保存长图。

注意事项：
- 截图区域尽量不超出滚动范围，避免框选整个窗口、全屏等。
- 截图区域不宜过小，建议高度大于等于50px。
- 截图区域中尽量避免存在多个滚动区域。
- 滚动截图区域中尽量不包含大量重复内容，如空白区域、相同颜色、大段相同文字等。
- 滚动截图区域中尽量不包含动图、视频、水印、纯色背景等信息。
- 滚动操作时保持垂直方向偏移，避免滑动滚动条。
- 滚动操作时尽量不要切换窗口。
- 滚动操作速度不宜过快。


### 识别文字

对于截图区域中无法复制的内容，可以使用识别文字功能提取文字。

1. 选择截图区域后，单击工具栏上的文字识别按钮 ![ocr](../common/ocr.svg)，弹出图文识别窗口，识别图片中的文字。
2. 您可以对识别出的文字进行编辑、复制或保存为TXT等操作。

![ocr](fig/ocr.png)

### 贴图

通过贴图功能将截图变为桌面浮窗，方便您快速查阅、对比和梳理信息。

选择需要截取的区域后，单击工具栏上的贴图按钮 ![icon](../common/pin.svg)，截图浮于桌面上即为贴图，您可以对贴图进行如下操作：

- 通过鼠标移动图片的位置。
- 将鼠标指针置于图片上，向上或向下滚动鼠标滚轮等比缩放图片。
- 单击按钮 ![icon](../common/done.svg) 保存图片。

![0|贴图](fig/pin_pic.png)

### AI截图

在截图工具栏中，单击按钮<img src="../common/bar_ai.svg" alt="AI截图" class="inline" />展开AI菜单，您可以：

- 选择 **解释**，让**AI随航**为您解释截图中的文字；
- 选择 **总结**，让**AI随航**为您总结截图中的文字；
- 选择 **翻译**，让**AI随航**为您翻译截图中的文字；
- 选择 **问问AI**，直接唤起**小U助手**并将截图发送至对话框，以进行更深入的对话。

![0|AI截图](fig/ai_tools.png)

### 延时截图

您可以通过延时截图功能，捕捉特定状态或时间下的屏幕内容。

- 使用快捷键 **Ctrl** + ![icon](../common/Print.svg)，启动延时截图。
- 在终端中执行命令 “deepin-screen-recorder -d n”，其中n表示以秒为单位的延时时长。 

### 设置

您还可以在截图工具栏中，单击按钮<img src="../common/bar_settings.svg" alt="设置" class="inline" />，展开设置菜单：

![0|option](fig/screen_setting.png)

- 选择**无边框/投影/外边框/样机**，设置图片边框效果。
- 选择 **PNG/JPG/BMP**，设置图片格式。
- 选择 **显示光标**，光标在截图区域内，截图生成图片时会同时显示光标所在的位置和样式。

### 保存截图

单击工具栏中的按钮![icon](../common/bar_save.svg)或单击鼠标右键选择 **保存**，可将截图保存在本地，单击该按钮右侧的小图标![icon](../common/dropdown.svg)展开选项可设置保存规则。

- 选择 **每次询问**，可在每次点击保存按钮后选择保存路径。
- 选择 **指定位置** > **桌面**，将图片保存到桌面。
- 选择 **指定位置** > **图片**，截图文件自动保存到图片文件夹目录下。
- 选择 **指定位置** > **保存时选择位置**，可在本次点击保存按钮后选择保存路径，下次截图保存时会默认保存至该路径，除非另行设置。

### 复制到剪贴板

单击工具栏中的按钮 ![icon](../common/done.svg)，或双击鼠标左键，或使用快捷键 **Ctrl + S**，将图片保存到剪贴板中。

录制提示: 打开应用 → 按用例步骤操作 → 每个操作后等待界面稳定 → 完成后按 q 结束

---
