#include "simview.h"
#include <cmath>

void SimView::wheelEvent(QWheelEvent *event)
{
    // 1. 获取滚轮滚动的角度delta
    // angleDelta().y() >0 表示向上滚（放大），<0表示向下滚（缩小）
    const double angle = event->angleDelta().y();

    // 2. 定义缩放因子（Zoom Factor）
    // 每次滚动缩放80%
    double scaleFactor = (angle > 0) ? 1.1 : 0.9;

    // 3. 执行缩放
    // scale()是QGraphicsView的方法，叠加当前的变换矩阵
    this->scale(scaleFactor, scaleFactor);

    // 注意：默认情况下，Qt会以View的中心为锚点缩放
    // 如果想以鼠标为中心缩放，可以在 setupUi 中设置 setTransformationAnchor(AnchorUnderMouse)
}
