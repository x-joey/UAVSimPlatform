#include "pathitem.h"
#include <QPainter>
#include <QPen>
PathItem::PathItem() {}

PathItem::PathItem(const QVector<QPointF> &path, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_path(path)
{
    // 计算边界矩形：QGraphicsView的一个重要优化
    // 计算图元占据多大空间，屏幕外即可不绘制
    if (!m_path.isEmpty()) {
        // 使用QPainterPath来计算边界，确保覆盖所有点
        QPainterPath p;
        p.addPolygon(QPolygonF(m_path));
        m_bounds = p.boundingRect();
    }
    setZValue(-1);   // 设置Z值，确保航迹在无人机（UavItem）下方绘制
}

QRectF PathItem::boundingRect() const
{
    // 返回缓存的边界矩形
    // 稍微放大一点边界，确保画笔在边界内
    return m_bounds.adjusted(-10, -10, 10, 10);
}

void PathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (m_path.isEmpty())
        return;

    // 设置画笔：虚线、红色、宽度为2
    QPen pen(Qt::DashLine);
    pen.setColor(Qt::red);
    pen.setWidth(2);
    painter->setPen(pen);

    // 绘制线段：将QVector<QPointF>（航迹点）绘制为折线
    painter->drawPolyline(m_path);

    // 绘制航迹点
    pen.setColor(Qt::darkRed);
    pen.setWidth(4);
    painter->setPen(pen);
    for (const auto &point : m_path) {
        // 绘制一个点
        painter->drawPoint(point);
    }
}
