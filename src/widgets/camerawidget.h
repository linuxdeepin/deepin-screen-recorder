#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>

class CameraWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CameraWidget(QWidget *parent = nullptr);
    void setRecordRect(int x, int y, int width, int height);
    void showAt(QPoint pos);
signals:

public slots:
protected:
    void enterEvent(QEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
private:
    int recordX;
    int recordY;
    int recordWidth;
    int recordHeight;
    bool m_move;
    QPoint m_startPoint;
    QPoint m_windowTopLeftPoint;
    QPoint m_windowTopRightPoint;
    QPoint m_windowBottomLeftPoint;
};

#endif // CAMERAWIDGET_H
