#ifndef RECORDERREGION_H
#define RECORDERREGION_H

#include <QPainter>
#include <QDesktopWidget>
#include <QPaintEvent>

#include "widgets/camerawidget.h"
#include "widgets/keybuttonwidget.h"

class RecorderRegionShow : public DWidget
{ 
    Q_OBJECT
public:
    RecorderRegionShow();
    virtual ~RecorderRegionShow();
    void initCameraInfo(const CameraWidget::Position position, const QSize size);
    void showKeyBoardButtons(const QString &key);
    void updateKeyBoardButtonStyle();

protected:
    void paintEvent(QPaintEvent *event);
    void updateMultiKeyBoardPos();


private:
    QPainter* m_painter;
    CameraWidget* m_cameraWidget;
    QList<KeyButtonWidget *> m_keyButtonList;
};
#endif
