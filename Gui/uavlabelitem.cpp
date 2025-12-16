#include "uavlabelitem.h"

#include <QDebug>
#include <QFont>
#include <QFontMetricsF>
#include <QStyleOptionGraphicsItem>
UavLabelItem::UavLabelItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
    updateBounds();
}

void UavLabelItem::setUavScenePos(const QPointF &pos)
{
    m_uavScenePos = pos;
    if (!m_hasCustomOffset) {
        m_offset = QPointF(20.0, 20.0);
    }

    m_updatingFromUav = true;
    setPos(m_uavScenePos + m_offset);
    m_updatingFromUav = false;

    updateBounds();
}

void UavLabelItem::setShowInfo(bool show)
{
    if (m_showInfo == show)
        return;
    m_showInfo = show;
    update();
}

void UavLabelItem::setInfoText(const QString &text)
{
    if (m_infoText == text)
        return;
    m_infoText = text;
    updateBounds();
    update();
}

QRectF UavLabelItem::boundingRect() const
{
    return m_bounds;
}

void UavLabelItem::updateBounds()
{
    QFont         font;
    QFontMetricsF fm(font);

    QString idText   = QString::number(m_id);
    QRectF  textRect = fm.boundingRect(idText);

    QRectF infoRect;
    if (m_showInfo && !m_infoText.isEmpty()) {
        infoRect = fm.boundingRect(m_infoText);
    }

    // 文本区域：ID 在上，信息在下
    qreal width  = qMax(textRect.width(), infoRect.width());
    qreal height = textRect.height() + (m_showInfo && !m_infoText.isEmpty() ? infoRect.height() + 4 : 0);

    // 线的另一端在无人机处（场景坐标），需要转换到本地坐标
    QPointF uavLocal = mapFromScene(m_uavScenePos);

    QRectF textBounds(QPointF(0, 0), QSizeF(width + 8, height + 8));
    QRectF lineBounds = QRectF(QPointF(0, 0), uavLocal).normalized().adjusted(-4, -4, 4, 4);

    m_bounds = textBounds.united(lineBounds);
}

void UavLabelItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setRenderHint(QPainter::Antialiasing, true);

    QFont font;
    painter->setFont(font);
    QFontMetricsF fm(font);

    QString idText = QString::number(m_id);
    QRectF  idRect = fm.boundingRect(idText);
    idRect.moveTopLeft(QPointF(4, 4));

    // 背景
    painter->setBrush(QColor(255, 255, 255, 220));
    painter->setPen(Qt::NoPen);
    QRectF bgRect = idRect.adjusted(-2, -2, 2, 2);
    if (m_showInfo && !m_infoText.isEmpty()) {
        QRectF infoRect = fm.boundingRect(m_infoText);
        infoRect.moveTopLeft(QPointF(4, idRect.bottom() + 4));
        bgRect = bgRect.united(infoRect.adjusted(-2, -2, 2, 2));
    }
    painter->drawRoundedRect(bgRect, 3, 3);

    //    qDebug() << "画笔颜色:" << painter->pen().color();
    //    qDebug() << "背景颜色:" << painter->brush().color();

    // 绘制 ID 文本
    painter->setPen(Qt::black);
    painter->drawText(idRect.topLeft() + QPointF(0, fm.ascent()), idText);

    // 如有需要，绘制信息文本
    if (m_showInfo && !m_infoText.isEmpty()) {
        QRectF infoRect = fm.boundingRect(m_infoText);
        infoRect.moveTopLeft(QPointF(4, idRect.bottom() + 4));
        painter->drawText(infoRect.topLeft() + QPointF(0, fm.ascent()), m_infoText);
    }
    qDebug() << "画笔颜色:" << painter->pen().color();
    qDebug() << "背景颜色:" << painter->brush().color();
    // 绘制虚线（从标签中心到底层无人机位置）
    painter->setPen(QPen(Qt::darkGray, 1, Qt::DashLine));
    QPointF uavLocal    = mapFromScene(m_uavScenePos);
    QPointF labelCenter = bgRect.center();
    painter->drawLine(labelCenter, uavLocal);
}

QVariant UavLabelItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && !m_updatingFromUav) {
        const QPointF newPos = value.toPointF();
        m_offset             = newPos - m_uavScenePos;
        m_hasCustomOffset    = true;
    }

    if (change == ItemPositionHasChanged) {
        prepareGeometryChange();
        updateBounds();
    }

    return QGraphicsItem::itemChange(change, value);
}
