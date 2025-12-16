/**
 * @file flightpath.h
 * @brief 航迹类头文件（Gui模块）
 * @details 用于生成和管理匀速直线运动航迹
 *          注意：此文件位于Gui模块，与Service模块中的FlightPath类不同
 */

#ifndef FLIGHTPATH_H
#define FLIGHTPATH_H

#include <QObject>

/**
 * @class FlightPath
 * @brief 航迹类（Gui模块）
 * @details 用于生成和管理匀速直线运动航迹
 *          设计原因：提供简单的航迹生成功能，支持基础的运动模式
 *          注意：当前实现较为简单，未来可扩展为支持更复杂的航迹生成算法
 *          注意：此文件位于Gui模块，与Service模块中的FlightPath类不同
 */
class FlightPath
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @details 初始化航迹对象
     */
    FlightPath();
};

#endif // FLIGHTPATH_H
