/**
 * @file simview.h
 * @brief 仿真视图类头文件
 * @details 自定义图形视图，支持鼠标滚轮缩放功能
 */

#ifndef SIMVIEW_H
#define SIMVIEW_H

#include <QGraphicsView>
#include <QObject>
#include <QWheelEvent>

/**
 * @class SimView
 * @brief 仿真视图类
 * @details 自定义图形视图，支持鼠标滚轮缩放功能
 *          设计原因：
 *          1. 缩放功能：支持鼠标滚轮缩放，提升用户体验
 *          2. 自定义交互：扩展基类功能，支持更多交互方式
 *          提升：
 *          - 用户体验：支持缩放操作，便于观察细节
 *          - 扩展性：易于添加更多交互功能（如平移、旋转等）
 */
class SimView : public QGraphicsView
{
    Q_OBJECT
public:
    /**
     * @brief 使用基类构造函数
     * @details 继承QGraphicsView的所有构造函数
     *          提升：简化代码，避免重复定义构造函数
     */
    using QGraphicsView::QGraphicsView;

protected:
    /**
     * @brief 处理鼠标滚轮事件
     * @param event 滚轮事件对象
     * @details 实现鼠标滚轮缩放功能
     *          算法：向上滚动放大（1.1倍），向下滚动缩小（0.9倍）
     *          提升：支持直观的缩放操作，提升用户体验
     */
    void wheelEvent(QWheelEvent *event) override;
};

#endif   // SIMVIEW_H
