/**
 * @file flightpath.h
 * @brief 航迹类头文件
 * @details 用于生成和管理匀速直线运动航迹
 *          注意：此文件位于Service模块，与Gui模块中的FlightPath类不同
 */

#ifndef FLIGHTPATH_H
#define FLIGHTPATH_H

#include <QList>
#include <QPointF>

/**
 * @class FlightPath
 * @brief 航迹类
 * @details 用于生成和管理匀速直线运动航迹
 *          设计原因：提供简单的航迹生成功能，支持基础的运动模式
 *          注意：当前实现较为简单，未来可扩展为支持更复杂的航迹生成算法
 */
class FlightPath
{
public:
    /**
     * @brief 构造函数
     * @details 初始化航迹对象
     */
    FlightPath();
    
    // TODO: 生成匀速直线运动航迹的方法（待实现）

private:
    /**
     * @brief 存储航迹点序列
     * @details 使用QList存储航迹点，支持动态添加和删除
     *          注意：当前未使用，保留用于未来扩展
     */
    QList<QPointF> m_path;
};

#endif // FLIGHTPATH_H
