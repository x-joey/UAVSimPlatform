#ifndef SIMSCENE_H
#define SIMSCENE_H
#include <QGraphicsScene>
#include <QPainter>
#include <QRectF>

class SimScene : public QGraphicsScene
{
    Q_OBJECT
public:
    using QGraphicsScene::QGraphicsScene;

protected:
    // 重写背景绘制函数
    // rect:当前视图可见的区域（Qt 只会让我们画屏幕上能看到的部分，优化性能）
    void drawBackground(QPainter *painter, const QRectF &rect) override;
};

#endif   // SIMSCENE_H
