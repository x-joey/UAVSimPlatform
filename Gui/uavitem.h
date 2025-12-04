#ifndef UAVITEM_H
#define UAVITEM_H
#pragma once
#include <QGraphicsItem>
#include <QPainter>

// 简单的无人机图元，负责在场景中绘制一个小圆点
class UavItem : public QGraphicsItem
{
public:
    explicit UavItem(QGraphicsItem *parent = nullptr);

    // 标识该图元对应的无人机 ID，便于点击时识别
    void  setId(int id) { m_id = id; }
    int   id() const { return m_id; }

    // 返回图元本身的包围矩形，Qt用它来优化绘图和碰撞检测
    QRectF boundingRect() const override;
    // paint() 实际的绘图函数
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    static constexpr qreal UAV_RADIUS = 5.0;

private:
    int m_id = -1;
};

#endif   // UAVITEM_H
