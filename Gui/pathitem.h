#ifndef PATHITEM_H
#define PATHITEM_H
#pragma once
#include <QGraphicsItem>
#include <QPointF>
#include <QVector>
// 继承QGraphicsItem, 才能添加到QGraphicsScene中
class PathItem : public QGraphicsItem
{
public:
    PathItem();
    PathItem(const QVector<QPointF> &path, QGraphicsItem *parent = nullptr);
    // 返回图元本身的包围矩形，Qt用它来优化绘图和碰撞检测
    QRectF boundingRect() const override;
    // paint() 实际的绘图函数
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QVector<QPointF> m_path;     // 存储航迹点
    QRectF           m_bounds;   // 缓存边界矩形
};

#endif   // PATHITEM_H
