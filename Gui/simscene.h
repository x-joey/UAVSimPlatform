#ifndef SIMSCENE_H
#define SIMSCENE_H
#include <QGraphicsScene>
#include <QPainter>
#include <QRectF>

// 自定义场景，负责绘制网格背景，并在点击图元时发出信号
class SimScene : public QGraphicsScene
{
    Q_OBJECT
public:
    using QGraphicsScene::QGraphicsScene;

    // 设置是否显示网格背景
    void setShowGrid(bool show) { m_showGrid = show; update(); }
    bool showGrid() const { return m_showGrid; }

signals:
    // 当用户点击某个无人机图元时，发出对应的无人机 ID
    void uavClicked(int uavId);

protected:
    // 重写背景绘制函数
    // rect:当前视图可见的区域（Qt 只会让我们画屏幕上能看到的部分，优化性能）
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    // 处理鼠标点击，识别被点击的图元（UavItem 或 UavLabelItem）
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    bool m_showGrid = true;  // 默认显示网格
};

#endif   // SIMSCENE_H
