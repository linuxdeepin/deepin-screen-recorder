#include "camerawidget.h"
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>

CameraWidget::CameraWidget(QWidget *parent) : QWidget(parent)
{
   setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint | Qt::Window);
   setFocusPolicy(Qt::StrongFocus);
       setMouseTracking(true);
       setAcceptDrops(true);
   resize(50, 50);
}

void CameraWidget::setRecordRect(int x, int y, int width, int height)
{
    this->recordX = x;
    this->recordY = y;
    this->recordWidth = width;
    this->recordHeight = height;
}

void CameraWidget::showAt(QPoint pos)
{
   if (!isVisible())
       this->show();
   move(pos.x(),pos.y());
}
void CameraWidget::enterEvent(QEvent *e)
{
    qApp->setOverrideCursor(Qt::ArrowCursor);
    qDebug()<<"CameraWidget enterEvent";
}
void CameraWidget::mousePressEvent(QMouseEvent *event)
{
    //当鼠标左键点击时.
    if (event->button() == Qt::LeftButton)
    {
        qDebug()<<"CameraWidget mousePressEvent";
        m_move = true;
        //记录鼠标的世界坐标.
        m_startPoint = event->globalPos();
        //记录窗体的世界坐标.
        m_windowTopLeftPoint = this->frameGeometry().topLeft();
        m_windowTopRightPoint = this->frameGeometry().topRight();
        m_windowBottomLeftPoint = this->frameGeometry().bottomLeft();
    }
}

void CameraWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        //移动中的鼠标位置相对于初始位置的相对位置.
        QPoint relativePos = event->globalPos() - m_startPoint;
        QRect recordRect = QRect(recordX, recordY, recordWidth, recordHeight);
        QPoint removePos = m_windowTopLeftPoint + relativePos;
        QPoint removeTopRightPos = m_windowTopRightPoint + relativePos;
        QPoint removeBottomLeftPos = m_windowBottomLeftPoint + relativePos;
        //然后移动窗体即可.
        if (recordRect.contains(removePos) && recordRect.contains(removeTopRightPos) && recordRect.contains(removeBottomLeftPos) ){
           this->move(removePos);
        }
    }
}

void CameraWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
       {
           //改变移动状态.
           m_move = false;
       }
}
