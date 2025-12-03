#include "uavitem.h"

UavItem::UavItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    setZValue(1);   // 设置Z值，确保无人机在航迹上方绘制
}

QRectF UavItem::boundingRect() const
{
    // 返回无人机（圆形）的包围矩形
    // QGraphicsItem的坐标是相对于自己的中心点的，所以是（-R，-R）到（R，R）
    return QRectF(-UAV_RADIUS, -UAV_RADIUS, 2 * UAV_RADIUS, 2 * UAV_RADIUS);
}

void UavItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{   // 设置画刷，蓝色实心填充
    painter->setBrush(QBrush(Qt::blue, Qt::SolidPattern));
    // 关闭轮廓线
    painter->setPen(Qt::NoPen);
    // 绘制一个圆形（Ellipse）, 参数是矩形的左上角、宽和高
    painter->drawEllipse(boundingRect());

    // 绘制方向指示线
    painter->setPen(QPen(Qt::white, 1));
    // 从中心（0，0）向右侧绘制一小段线段，模拟无人机头部
    painter->drawLine(0, 0, UAV_RADIUS, 0);
}
