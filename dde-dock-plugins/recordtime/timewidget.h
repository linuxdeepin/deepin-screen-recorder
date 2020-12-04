#ifndef TIMEWIDGET_H
#define TIMEWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QTime>
#include <QIcon>
#include <DWidget>
#include <com_deepin_dde_daemon_dock.h>
#include <DFontSizeManager>
#include <QBoxLayout>
#include <QLabel>

#define RECORDER_TIME_LEVEL_ICON_SIZE 32
#define RECORDER_TIME_VERTICAL_ICON_SIZE 28
#define RECORDER_TIME_LEVEL_SIZE "00000 00:00:00"
#define RECORDER_TIME_VERTICAL_SIZE "0000"
#define RECORDER_TIME_FONT DFontSizeManager::instance()->t8()

DWIDGET_USE_NAMESPACE
using DBusDock = com::deepin::dde::daemon::Dock;

class TimeWidget : public DWidget
{
    Q_OBJECT
public:
    explicit TimeWidget(DWidget *parent = nullptr);

    //---------------------------------------------------------------
    ~TimeWidget();

    //---------------------------------------------------------------
    bool enabled();
    void start();
    void stop();
    QSize sizeHint() const;

protected:
    void mousePressEvent(QMouseEvent *e);

private slots:
    void onTimeout();
    void onPositionChanged(int value);

private:
    const QPixmap loadSvg(const QString &fileName, const QSize &size) const;
    void paintEvent(QPaintEvent *e);

private:
    QTimer *m_timer;
    DBusDock *m_dockInter;
    QIcon *m_lightIcon;
    QIcon *m_shadeIcon;
    QIcon *m_currentIcon;
    QPixmap m_pixmap;
    QSize m_textSize;
    QTime m_baseTime;
    QString m_showTimeStr;
    bool m_bRefresh;
    int m_position;
    QBoxLayout *centralLayout;
};

#endif // TIMEWIDGET_H
