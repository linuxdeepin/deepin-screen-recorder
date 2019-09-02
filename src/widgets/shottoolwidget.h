#ifndef SHOTTOOLWIDGET_H
#define SHOTTOOLWIDGET_H

#include <QObject>
#include <DLabel>
#include <DStackedWidget>
DWIDGET_USE_NAMESPACE

class ShotToolWidget : public DStackedWidget
{
    Q_OBJECT
public:
    explicit ShotToolWidget(QWidget *parent = nullptr);
    ~ShotToolWidget();

    void initWidget();
    //截屏矩形按钮二级控件初始化
    void initRectLabel();
    //截屏圆形按钮二级控件初始化
    void initCircLabel();
    //截屏直线按钮二级控件初始化
    void initLineLabel();
    //截屏画笔按钮二级控件初始化
    void initPenLabel();
    //截屏文本按钮二级控件初始化
    void initTextLabel();

signals:

public slots:
    void switchContent(QString shapeType);

private:
    QLabel *m_rectSubTool;
    QLabel *m_circSubTool;
    QLabel *m_lineSubTool;
    QLabel *m_penSubTool;
    QLabel *m_textSubTool;
    QString m_currentType;
    bool m_arrowFlag;
};

#endif // SHOTTOOLWIDGET_H
