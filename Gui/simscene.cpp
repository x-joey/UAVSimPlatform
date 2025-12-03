#include "simscene.h"
#include <QPen>
#include <cmath>

void SimScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    // 1. 定义网格大小
    const int gridSize = 50;

    // 2. 获取当前可见区域的边界
    qreal left   = rect.left();
    qreal right  = rect.right();
    qreal top    = rect.top();
    qreal bottom = rect.bottom();

    // 3. 计算从哪里开始画线
    qreal startX = std::floor(left / gridSize) * gridSize;
    qreal startY = std::floor(top / gridSize) * gridSize;

    // 4. 设置画笔（浅灰色，极细）
    QPen pen(Qt::lightGray, 0);
    painter->setPen(pen);

    // 5. 绘制垂直线
    for (qreal x = startX; x <= right; x += gridSize) {
        painter->drawLine(QLineF(x, top, x, bottom));
    }

    // 6. 绘制水平线
    for (qreal y = startY; y <= bottom; y += gridSize) {
        painter->drawLine(QLineF(left, y, right, y));
    }

    // 7. 绘制坐标原点 (十字线)，方便定位
    QPen originPen(Qt::black, 1);
    painter->setPen(originPen);
    painter->drawLine(0, -10, 0, 10);   // Y轴标记
    painter->drawLine(-10, 0, 10, 0);   // X轴标记
}
