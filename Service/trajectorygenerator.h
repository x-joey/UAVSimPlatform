/**
 * @file trajectorygenerator.h
 * @brief 航迹生成器类头文件
 * @details 提供多种航迹生成算法，支持圆形、8字形和航点航迹
 *          采用静态方法设计，无需实例化即可使用，提升使用便利性
 */

#ifndef TRAJECTORYGENERATOR_H
#define TRAJECTORYGENERATOR_H

#include <QList>
#include <QPointF>
#include <QVector>

/**
 * @class TrajectoryGenerator
 * @brief 航迹生成器工具类
 * @details 提供多种航迹生成算法，用于创建无人机的飞行路径
 *          设计原因：
 *          1. 工具类设计：所有方法都是静态的，无需实例化，使用方便
 *          2. 算法集中管理：将航迹生成逻辑集中在一个类中，便于维护和扩展
 *          3. 参数化设计：支持自定义中心、半径、点数等参数，提升灵活性
 *          4. 数学精确性：使用标准数学公式，确保航迹的几何正确性
 *          提升：
 *          - 代码复用：避免在多个地方重复实现相同算法
 *          - 可维护性：集中管理，修改算法只需改一处
 *          - 可扩展性：易于添加新的航迹生成算法（如螺旋、椭圆等）
 */
class TrajectoryGenerator
{
public:
    /**
     * @brief 构造函数
     * @details 工具类通常不需要实例化，构造函数为空
     */
    TrajectoryGenerator();

    /**
     * @brief 生成圆形航迹
     * @param center 圆心坐标
     * @param radius 半径（像素或米）
     * @param pointsCount 生成点的数量，越多越平滑
     * @return 航迹点序列
     * @details 使用参数方程生成圆形航迹点
     *          数学公式：x = center_x + r * cos(θ), y = center_y + r * sin(θ)
     *          设计原因：圆形航迹是最基础的航迹模式，适用于巡逻、监视等场景
     *          提升：预分配内存避免频繁扩容，提升性能
     */
    static QVector<QPointF> createCirclePath(QPointF center, double radius, int pointsCount);

    /**
     * @brief 生成8字形航迹
     * @param center 中心坐标
     * @param radius 半径（像素或米）
     * @param pointsCount 生成点的数量，越多越平滑
     * @return 航迹点序列
     * @details 使用伯努利双纽线的简化参数方程生成8字形航迹
     *          数学公式：x = r * cos(t), y = r * sin(t) * cos(t)
     *          设计原因：8字形航迹展示复杂运动模式，适用于搜索、扫描等场景
     *          提升：增加航迹多样性，提升仿真真实感
     */
    static QVector<QPointF> createEightShapePath(QPointF center, double radius, int pointsCount);

    /**
     * @brief 根据航点列表生成航迹
     * @param waypoints 用户定义的关键航点序列
     * @param stepSize 插值步长（像素），控制点与点之间的密度
     * @return 插值后的完整航迹点序列
     * @details 在相邻航点之间进行线性插值，生成连续的航迹
     *          设计原因：支持用户自定义航迹，提升系统灵活性
     *          算法：计算两点间距离和方向，按步长插值生成中间点
     *          提升：支持任意形状的航迹，满足复杂任务需求
     */
    static QVector<QPointF> createPathFromWaypoints(const QVector<QPointF> &waypoints, double stepSize = 2.0);

public:
    /**
     * @brief 临时存储航迹点的容器（已废弃，保留以兼容旧代码）
     * @details 建议使用静态方法的返回值，而非此成员变量
     */
    QList<QPointF> m_path;
};

#endif   // TRAJECTORYGENERATOR_H
