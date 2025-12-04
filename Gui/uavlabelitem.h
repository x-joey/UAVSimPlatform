#ifndef UAVLABELITEM_H
#define UAVLABELITEM_H
#pragma once

#include <QGraphicsItem>
#include <QPainter>

// 可拖动的无人机标签图元：显示 ID，并用虚线连接到无人机位置
class UavLabelItem : public QGraphicsItem
{
public:
    explicit UavLabelItem(QGraphicsItem *parent = nullptr);

    void setId(int id) { m_id = id; }
    int  id() const { return m_id; }

    void     setName(const QString &name) { m_name = name; }
    QString  name() const { return m_name; }

    // 设置无人机在场景坐标系中的位置，用于画虚线
    void setUavScenePos(const QPointF &pos);

    // 是否在 ID 下方显示实时信息（由外部控制，比如点击时）
    void setShowInfo(bool show);
    bool showInfo() const { return m_showInfo; }

    // 由外部更新的实时信息文本（例如坐标等），只在 showInfo=true 时绘制
    void setInfoText(const QString &text);

    QRectF boundingRect() const override;
    void   paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    int      m_id        = -1;
    QString  m_name;
    QPointF  m_uavScenePos;    // 无人机在场景坐标中的位置
    QPointF  m_offset     = QPointF(20.0, 20.0);   // 标签相对无人机的偏移
    bool     m_showInfo   = false;
    bool     m_hasCustomOffset = false;
    bool     m_updatingFromUav = false;
    QString  m_infoText;
    QRectF   m_bounds;         // 缓存包围矩形

    void updateBounds();
};

#endif   // UAVLABELITEM_H


