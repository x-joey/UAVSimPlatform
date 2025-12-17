/**
 * @file DraggableHeaderView.cpp
 * @brief 可拖动列的表格头视图组件实现
 */

#include "DraggableHeaderView.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QTableWidget>

DraggableHeaderView::DraggableHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    // 设置section可移动（这是Qt内置功能，但我们要自定义）
    setSectionsMovable(false);   // 禁用内置移动，使用自定义实现
    setSectionsClickable(true);
    setHighlightSections(true);

    // 设置拖动时的鼠标样式
    setCursor(Qt::ArrowCursor);
}

void DraggableHeaderView::setColumnDraggingEnabled(bool enable)
{
    m_draggingEnabled = enable;
    if (!enable && m_isDragging) {
        // 如果正在拖动时禁用，结束拖动
        m_isDragging     = false;
        m_draggedColumn  = -1;
        m_targetColumn   = -1;
        setCursor(Qt::ArrowCursor);
        update();
    }
}

void DraggableHeaderView::mousePressEvent(QMouseEvent *event)
{
    if (!m_draggingEnabled || event->button() != Qt::LeftButton) {
        QHeaderView::mousePressEvent(event);
        return;
    }

    // 获取点击的列索引
    int logicalIndex = getLogicalIndexAt(event->pos());
    if (logicalIndex >= 0) {
        m_isDragging    = true;
        m_draggedColumn = logicalIndex;
        m_dragStartPos  = event->pos();
        m_currentPos    = event->pos();
        setCursor(Qt::ClosedHandCursor);

        qDebug() << "Started dragging column:" << logicalIndex;
    }

    // 仍然调用基类处理，保持正常的点击行为
    QHeaderView::mousePressEvent(event);
}

void DraggableHeaderView::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_draggingEnabled || !m_isDragging) {
        QHeaderView::mouseMoveEvent(event);
        return;
    }

    m_currentPos = event->pos();

    // 获取当前鼠标下的列索引
    int targetIndex = getLogicalIndexAt(event->pos());
    if (targetIndex >= 0 && targetIndex != m_draggedColumn) {
        m_targetColumn = targetIndex;
    }
    else {
        m_targetColumn = -1;
    }

    // 触发重绘以显示拖动指示器
    update();

    event->accept();
}

void DraggableHeaderView::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_draggingEnabled || !m_isDragging) {
        QHeaderView::mouseReleaseEvent(event);
        return;
    }

    if (m_targetColumn >= 0 && m_targetColumn != m_draggedColumn) {
        qDebug() << "Swapping columns:" << m_draggedColumn << "and" << m_targetColumn;

        // 交换列
        swapColumns(m_draggedColumn, m_targetColumn);

        // 发出信号
        emit columnSwapped(m_draggedColumn, m_targetColumn);
    }

    // 结束拖动
    m_isDragging    = false;
    m_draggedColumn = -1;
    m_targetColumn  = -1;
    setCursor(Qt::ArrowCursor);
    update();

    QHeaderView::mouseReleaseEvent(event);
}

void DraggableHeaderView::paintEvent(QPaintEvent *event)
{
    // 先绘制正常的表头
    QHeaderView::paintEvent(event);

    // 如果正在拖动，绘制拖动指示器
    if (m_isDragging) {
        QPainter painter(viewport());
        drawDragIndicator(&painter);
    }
}

int DraggableHeaderView::getLogicalIndexAt(const QPoint &pos) const
{
    return logicalIndexAt(pos);
}

void DraggableHeaderView::drawDragIndicator(QPainter *painter)
{
    if (!m_isDragging || m_draggedColumn < 0) {
        return;
    }

    painter->save();

    // 绘制被拖动列的半透明高亮
    int sectionPos  = sectionViewportPosition(m_draggedColumn);
    int draggedSectionSize = sectionSize(m_draggedColumn);

    QRect draggedRect;
    if (orientation() == Qt::Horizontal) {
        draggedRect = QRect(sectionPos, 0, draggedSectionSize, height());
    }
    else {
        draggedRect = QRect(0, sectionPos, width(), draggedSectionSize);
    }

    // 绘制被拖动列的高亮（半透明蓝色）
    painter->fillRect(draggedRect, QColor(100, 150, 255, 80));

    // 如果有目标列，绘制插入指示线
    if (m_targetColumn >= 0) {
        int targetPos = sectionViewportPosition(m_targetColumn);

        QPen indicatorPen(QColor(255, 100, 100), 3);
        painter->setPen(indicatorPen);

        if (orientation() == Qt::Horizontal) {
            // 在目标列的左侧或右侧绘制指示线
            int lineX = (m_targetColumn < m_draggedColumn) ? targetPos : (targetPos + sectionSize(m_targetColumn));
            painter->drawLine(lineX, 0, lineX, height());

            // 绘制箭头
            QPolygon arrow;
            arrow << QPoint(lineX, 5) << QPoint(lineX - 5, 0) << QPoint(lineX + 5, 0);
            painter->setBrush(QColor(255, 100, 100));
            painter->drawPolygon(arrow);

            arrow.clear();
            arrow << QPoint(lineX, height() - 5) << QPoint(lineX - 5, height()) << QPoint(lineX + 5, height());
            painter->drawPolygon(arrow);
        }
        else {
            // 垂直方向
            int lineY = (m_targetColumn < m_draggedColumn) ? targetPos : (targetPos + sectionSize(m_targetColumn));
            painter->drawLine(0, lineY, width(), lineY);

            // 绘制箭头
            QPolygon arrow;
            arrow << QPoint(5, lineY) << QPoint(0, lineY - 5) << QPoint(0, lineY + 5);
            painter->setBrush(QColor(255, 100, 100));
            painter->drawPolygon(arrow);

            arrow.clear();
            arrow << QPoint(width() - 5, lineY) << QPoint(width(), lineY - 5) << QPoint(width(), lineY + 5);
            painter->drawPolygon(arrow);
        }
    }

    painter->restore();
}

void DraggableHeaderView::swapColumns(int oldIndex, int newIndex)
{
    if (oldIndex == newIndex || oldIndex < 0 || newIndex < 0) {
        return;
    }

    // 获取父表格
    QTableWidget *table = qobject_cast<QTableWidget *>(parentWidget());
    if (!table) {
        qWarning() << "DraggableHeaderView: Parent is not a QTableWidget";
        return;
    }

    // 禁用排序以避免干扰
    bool wasSortingEnabled = table->isSortingEnabled();
    table->setSortingEnabled(false);

    // 交换两列的所有数据
    if (orientation() == Qt::Horizontal) {
        // 交换两列的所有数据
        int rowCount = table->rowCount();

        for (int row = 0; row < rowCount; ++row) {
            // 保存oldIndex列的数据
            QTableWidgetItem *item1 = table->takeItem(row, oldIndex);
            QTableWidgetItem *item2 = table->takeItem(row, newIndex);

            // 交换数据
            if (item2) {
                table->setItem(row, oldIndex, item2);
            }
            if (item1) {
                table->setItem(row, newIndex, item1);
            }
        }

        // 交换表头文本
        QString header1 = model()->headerData(oldIndex, Qt::Horizontal).toString();
        QString header2 = model()->headerData(newIndex, Qt::Horizontal).toString();
        model()->setHeaderData(oldIndex, Qt::Horizontal, header2);
        model()->setHeaderData(newIndex, Qt::Horizontal, header1);
    }

    // 恢复排序状态
    table->setSortingEnabled(wasSortingEnabled);

    qDebug() << "Columns swapped successfully:" << oldIndex << "<->" << newIndex;
}
