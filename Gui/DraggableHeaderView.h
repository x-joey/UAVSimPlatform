/**
 * @file DraggableHeaderView.h
 * @brief 可拖动列的表格头视图组件
 * @details 实现类似Excel的列拖动功能，可在任何项目中重用
 *          设计原因：提升用户体验，允许自定义列顺序
 *          提升：独立组件，易于集成到其他项目
 */

#ifndef DRAGGABLEHEADERVIEW_H
#define DRAGGABLEHEADERVIEW_H

#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <QPoint>

/**
 * @class DraggableHeaderView
 * @brief 支持列拖动的表头视图
 * @details 继承自QHeaderView，添加拖动列功能
 *          使用方法：
 *          @code
 *          QTableWidget *table = new QTableWidget();
 *          DraggableHeaderView *header = new DraggableHeaderView(Qt::Horizontal, table);
 *          table->setHorizontalHeader(header);
 *          @endcode
 */
class DraggableHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param orientation 表头方向（水平或垂直）
     * @param parent 父对象
     */
    explicit DraggableHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~DraggableHeaderView() override = default;

    /**
     * @brief 启用/禁用列拖动功能
     * @param enable true启用，false禁用
     */
    void setColumnDraggingEnabled(bool enable);

    /**
     * @brief 获取列拖动功能状态
     * @return true已启用，false已禁用
     */
    bool isColumnDraggingEnabled() const { return m_draggingEnabled; }

signals:
    /**
     * @brief 列交换完成信号
     * @param oldIndex 原列索引
     * @param newIndex 新列索引
     */
    void columnSwapped(int oldIndex, int newIndex);

protected:
    /**
     * @brief 鼠标按下事件
     * @param event 鼠标事件
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief 鼠标移动事件
     * @param event 鼠标事件
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief 鼠标释放事件
     * @param event 鼠标事件
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /**
     * @brief 绘制事件
     * @param event 绘制事件
     */
    void paintEvent(QPaintEvent *event) override;

private:
    /**
     * @brief 获取鼠标位置对应的列索引
     * @param pos 鼠标位置
     * @return 列索引，-1表示无效
     */
    int getLogicalIndexAt(const QPoint &pos) const;

    /**
     * @brief 绘制拖动指示器
     * @param painter 画笔
     */
    void drawDragIndicator(QPainter *painter);

    /**
     * @brief 交换两列
     * @param oldIndex 原列索引
     * @param newIndex 新列索引
     */
    void swapColumns(int oldIndex, int newIndex);

private:
    bool   m_draggingEnabled = true;    ///< 是否启用拖动
    bool   m_isDragging      = false;   ///< 是否正在拖动
    int    m_draggedColumn   = -1;      ///< 被拖动的列索引
    int    m_targetColumn    = -1;      ///< 目标列索引
    QPoint m_dragStartPos;              ///< 拖动开始位置
    QPoint m_currentPos;                ///< 当前鼠标位置
};

#endif   // DRAGGABLEHEADERVIEW_H
