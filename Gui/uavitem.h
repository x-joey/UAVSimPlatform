#ifndef UAVITEM_H
#define UAVITEM_H
#pragma once
#include <QGraphicsItem>
#include <QPainter>
class UavItem : public QGraphicsItem
{
public:
    UavItem(QGraphicsItem *parent = nullptr);
    // 返回图元本身的包围矩形，Qt用它来优化绘图和碰撞检测
    QRectF boundingRect() const override;
    // paint() 实际的绘图函数
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    static constexpr qreal UAV_RADIUS = 5.0;
};

#endif   // UAVITEM_H
