/**
 * @file uavitem.h
 * @brief 无人机图元类头文件
 * @details 定义在场景中显示无人机的图形图元
 */

#ifndef UAVITEM_H
#define UAVITEM_H
#pragma once
#include <QGraphicsItem>
#include <QPainter>

/**
 * @class UavItem
 * @brief 无人机图元类
 * @details 简单的无人机图元，负责在场景中绘制一个小圆点表示无人机
 *          设计原因：
 *          1. 可视化：将UAV模型转换为可视化的图形元素
 *          2. 交互性：支持点击识别，便于用户选择
 *          3. 性能：使用简单的圆形绘制，提升渲染性能
 *          提升：
 *          - 可视化：直观显示无人机位置
 *          - 交互性：支持点击选择，提升用户体验
 *          - 性能：简单图形，渲染开销小
 */
class UavItem : public QGraphicsItem
{
public:
    /**
     * @brief 构造函数
     * @param parent 父图元指针
     * @details 初始化无人机图元，设置Z值确保在航迹上方绘制
     */
    explicit UavItem(QGraphicsItem *parent = nullptr);

    /**
     * @brief 设置无人机ID
     * @param id 无人机唯一标识符
     * @details 标识该图元对应的无人机ID，便于点击时识别
     *          提升：支持快速识别和查找对应的UAV模型
     */
    void setId(int id) { m_id = id; }

    /**
     * @brief 获取无人机ID
     * @return 无人机唯一标识符
     */
    int id() const { return m_id; }

    /**
     * @brief 返回图元的包围矩形
     * @return 包围矩形
     * @details Qt用它来优化绘图和碰撞检测
     *          提升：只绘制可见区域，提升渲染性能
     */
    QRectF boundingRect() const override;

    /**
     * @brief 绘制图元
     * @param painter 画笔对象
     * @param option 样式选项
     * @param widget 绘制目标窗口（可选）
     * @details 实际的绘图函数，绘制蓝色圆形表示无人机
     *          提升：使用简单图形，提升渲染性能
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    /**
     * @brief 无人机半径常量
     * @details 定义无人机图元的显示半径，单位像素
     */
    static constexpr qreal UAV_RADIUS = 5.0;

private:
    /**
     * @brief 无人机ID
     * @details 用于标识该图元对应的无人机
     *          默认值为-1，表示未设置
     */
    int m_id = -1;
};

#endif   // UAVITEM_H
