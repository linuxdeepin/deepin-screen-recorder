#include "previewwidget.h"
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>

PreviewWidget::PreviewWidget(const QRect &rect, QWidget *parent) : QWidget(parent), m_previewRect(rect)
{
    //m_RecordRect = rect;
    m_recordHeight = rect.height();
    m_recordWidth = rect.width();
    m_recordX = rect.x();
    m_recordY = rect.y();
    m_maxHeight = QApplication::desktop()->screen()->geometry().height() * 7 / 10;
    //计算位置

}
//初始化位置，大小
void PreviewWidget::initPreviewWidget()
{
    m_previewRect = previewGeomtroy();
    setGeometry(m_previewRect);
}

void PreviewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    //paint
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawImage(rect(), m_currentPix);
}
//设置位置状态
void PreviewWidget::setPreviewWidgetStatusPos(int statusPos)
{
    m_StatusPos = statusPos;
}
//更新图片
void PreviewWidget::updateImage(const QImage &image)
{
    int previewHight = image.height() >= m_maxHeight ? m_maxHeight : image.height();//是否超出最大高度
    int hightDiff_t = previewHight - m_previewRect.height(); //高度差
    //判断向上是否超出屏幕外
    int previewY = m_previewRect.y() - hightDiff_t;
    if (previewY <= 0) {
        previewY = 0;
        previewHight = m_recordY + m_recordHeight;//设置此时的预览高度=捕捉区域的高度+y轴坐标
    }
    qDebug() << "1   m_previewRect.x(): " << m_previewRect.x() << ", m_previewRect.y(): " << m_previewRect.y() << ", m_previewRect.width(): " << m_previewRect.width() << ", m_previewRect.height(): " << m_previewRect.height();
    m_previewRect.setY(previewY);//重新设置y坐标
    m_previewRect.setHeight(previewHight);//重新设置预览高度
    //当预览高度大于最大高度、或者向上到顶时，进行图片缩放，
    if (previewHight == m_maxHeight || previewY == 0) {
        QImage tempImage = image.scaled(m_previewRect.width(), previewHight, Qt::KeepAspectRatio, Qt::SmoothTransformation);//以预览框的宽高等比例缩放
        int widthDiff_t = m_previewRect.width() - tempImage.width();//宽度差
        if (m_StatusPos == 1) {//预览框在左
            m_previewRect.setX(m_previewRect.x() + widthDiff_t);//重新设置x坐标
        } else if (m_StatusPos == 0) {//预览框在右
            m_previewRect.setX(m_previewRect.x());
        } else {//预览框在内部
            m_previewRect.setX(m_previewRect.x() + widthDiff_t);
        }
        m_previewRect.setWidth(tempImage.width());//重新设置预览宽度
    }

    qDebug() << "2  m_previewRect.x(): " << m_previewRect.x() << ", m_previewRect.y(): " << m_previewRect.y() << ", m_previewRect.width(): " << m_previewRect.width() << ", m_previewRect.height(): " << m_previewRect.height();
    setGeometry(m_previewRect);
    m_currentPix = image;
    update();
}

//根据捕捉区域大小初始化预览框位置大小
QRect PreviewWidget::previewGeomtroy()
{
    //考虑双屏，todo
    QRect rt;
    QDesktopWidget *desktop = QApplication::desktop();
    QRect mainRect = desktop->screen()->geometry();
    int propMaxHeight = mainRect.height() * 7 / 10 ; //最大高度
    int propMaxWidth = mainRect.width() / 4 ; //屏幕宽度50%，除以2，最大宽度
    int previewHeight = 0; //预览高度
    int previewWidth = 0; //预览宽度

    if (m_recordHeight <= propMaxHeight && m_recordWidth <= propMaxWidth) {
        previewHeight = m_recordHeight;
        previewWidth = m_recordWidth;
        rt = calculatePreviewPosition(previewWidth, previewHeight);
    } else if (m_recordHeight <= propMaxHeight && m_recordWidth > propMaxWidth) {
        previewHeight = propMaxWidth * m_recordHeight / m_recordWidth;;
        previewWidth = propMaxWidth;
        rt = calculatePreviewPosition(previewWidth, previewHeight);
    } else if (m_recordHeight > propMaxHeight && m_recordWidth <= propMaxWidth) {
        previewHeight = propMaxHeight;
        previewWidth = propMaxHeight * m_recordWidth / m_recordHeight;
        rt = calculatePreviewPosition(previewWidth, propMaxHeight);
    } else if (m_recordHeight > propMaxHeight && m_recordWidth > propMaxWidth) {
        previewHeight = propMaxWidth * m_recordHeight / m_recordWidth; //假设宽度为最大
        previewWidth = propMaxHeight * m_recordWidth / m_recordHeight;//假设高度为最大
        if (previewHeight <= propMaxHeight) {
            previewWidth = propMaxWidth;
            rt = calculatePreviewPosition(previewWidth, previewHeight);
        } else if (previewWidth <= propMaxWidth) {
            previewHeight = propMaxHeight;
            rt = calculatePreviewPosition(previewWidth, previewHeight);
        }
    }
    qDebug() << "propMaxHeight: " << propMaxHeight << ", propMaxWidth: " << propMaxWidth;
    qDebug() << "previewHeight: " << previewHeight << ", previewWidth: " << previewWidth;
    qDebug() << "rt.x(): " << rt.x() << ", rt.y(): " << rt.y() << ", rt.width(): " << rt.width() << ", rt.height(): " << rt.height();
    return rt;
}
//判定在左还是在右
QRect PreviewWidget::calculatePreviewPosition(int previewWidth, int previewHeight)
{
    QRect rt;
    int previewY = m_recordY + m_recordHeight - previewHeight;
    int rightX = m_recordX + m_recordWidth + 23; //根据捕捉区域计算预览向右的起始x
    int leftX = m_recordX - previewWidth - 23; //根据捕捉区域计算预览向左的起始x
    qDebug() << "m_screenWidth: " << m_screenWidth ;
    int rightMargin = m_screenWidth - rightX; //右边距
    int leftMargin = m_recordX; //左边距
    qDebug() << "leftMargin: " << leftMargin << ", rightMargin: " << rightMargin;
    //判断位置是左还是右
    if (rightMargin >= leftMargin && rightMargin >= previewWidth + 1) {
        rt.setX(rightX);
        rt.setY(previewY);
        rt.setWidth(previewWidth);
        rt.setHeight(previewHeight);
        setPreviewWidgetStatusPos(0);//设置位置状态
    } else if (rightMargin < leftMargin && leftMargin >= previewWidth + 1) {
        rt.setX(leftX);
        rt.setY(previewY);
        rt.setWidth(previewWidth);
        rt.setHeight(previewHeight);
        setPreviewWidgetStatusPos(1);
    } else {
        int nX = m_recordX + m_recordWidth - 23 - previewWidth;//根据捕捉区域计算内部预览的起始x
        int ny = m_recordY + m_recordHeight - previewHeight - 23;//根据捕捉区域计算内部预览的起始y
        rt.setX(nX);
        rt.setY(ny);
        rt.setWidth(previewWidth);
        rt.setHeight(previewHeight);
        setPreviewWidgetStatusPos(2);
    }
    return rt;
}
//设置屏幕宽度
void PreviewWidget::setScreenWidth(int screenWidth)
{
    m_screenWidth = screenWidth;
}
