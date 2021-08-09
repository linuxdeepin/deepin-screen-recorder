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
    void updateImage(const QImage &image)
    {
        int himage = image.height() > m_maxHeight ? m_maxHeight : image.height();
        //判断向上是否超出屏幕外todo
        int previewY = m_previewRect.y() + m_previewRect.height() - himage;
        if (previewY < 0) {
            //to do...
        }
        qDebug() << __FUNCTION__ << "Y:" << previewY;
        m_previewRect.setY(previewY);
        m_previewRect.setSize(QSize(image.width(), himage));
        setGeometry(m_previewRect);
        m_currentPix = image;
        update();
    }

    void paintEvent(QPaintEvent *event) override;

    void setRect(const QRect &rect);
signals:

public slots:

private:
    QImage m_currentPix = QImage("/home/hjlt/Desktop/t.png");
    QRect m_previewRect;
    int m_maxHeight = 0;
};

#endif // SCANPAGEWIDGET_H
