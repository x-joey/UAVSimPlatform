#ifndef SIMVIEW_H
#define SIMVIEW_H

#include <QGraphicsView>
#include <QObject>
#include <QWheelEvent>
class SimView : public QGraphicsView
{
    Q_OBJECT
public:
    // 使用using 继承父类构造函数
    using QGraphicsView::QGraphicsView;

protected:
    void wheelEvent(QWheelEvent *event) override;
};

#endif   // SIMVIEW_H
