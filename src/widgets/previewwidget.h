#ifndef SCANPAGEWIDGET_H
#define SCANPAGEWIDGET_H

#include <QWidget>
#include <QDebug>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <qglobal.h>

class PreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PreviewWidget(const QRect &rect, QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override;
    //更新图片
    void updateImage(const QImage &image);
    //标记预览位置在左还是在右，0:右，1：左, 2内部
    void setPreviewWidgetStatusPos(int statusPos);
    //计算预览位置或大小
    QRect previewGeomtroy();
    //计算预览位置
    QRect calculatePreviewPosition(int previewWidth, int previewHeight);
    //设置屏幕宽度
    void setScreenWidth(int screenWidth);
    //初始化预览位置大小
    void initPreviewWidget();

signals:

public slots:

private:
    QImage m_currentPix = QImage("/home/hjlt/Desktop/t.png");
    QRect m_previewRect; //预览区域
    int m_maxHeight = 0; //预览最大高度
    int m_maxWidth = 0;//预览最大宽度
    int m_StatusPos = 0; //位置状态

    int m_recordHeight = 0; //捕捉区域高度
    int m_recordWidth = 0; //捕捉区域宽度
    int m_recordX = 0;//捕捉区域x坐标
    int m_recordY = 0;//捕捉区域y坐标
    int m_screenWidth = 0;//屏幕宽度
    //预览位置
    enum PostionStatus {
        RIGHT = 0,
        LEFT,
        INSIDE
    };
};

#endif // SCANPAGEWIDGET_H
