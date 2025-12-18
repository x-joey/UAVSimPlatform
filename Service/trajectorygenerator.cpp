/**
 * @file trajectorygenerator.cpp
 * @brief 航迹生成器类实现文件
 */

#include "trajectorygenerator.h"
#include <QVector2D>
#include <cmath>
#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

TrajectoryGenerator::TrajectoryGenerator() {}

QVector<QPointF> TrajectoryGenerator::createCirclePath(QPointF center, double radius, int pointsCount)
{
    QVector<QPointF> path;

    // 预分配内存，这是 C++ 的性能优化习惯
    // 避免 vector 在添加元素时频繁扩容，减少内存重新分配的开销
    // 提升：当pointsCount较大时，可显著提升性能（减少多次内存分配和拷贝）
    path.reserve(pointsCount);

    // 遍历生成每个航迹点
    for (int i = 0; i < pointsCount; ++i) {
        // 计算当前角度：均匀分布在0到2π之间
        // 使用整数除法确保角度均匀分布，避免浮点误差累积
        double angle = 2 * M_PI * i / pointsCount;

        // 圆形参数方程：将极坐标转换为笛卡尔坐标
        // 数学公式:
        // x = center_x + r * cos(θ)
        // y = center_y + r * sin(θ)
        // 提升：使用标准数学公式，确保几何正确性和数值稳定性
        double x = center.x() + radius * std::cos(angle);
        double y = center.y() + radius * std::sin(angle);

        path.append(QPointF(x, y));
    }

    return path;
}

QVector<QPointF> TrajectoryGenerator::createEightShapePath(QPointF center, double radius, int pointsCount)
{
    QVector<QPointF> path;
    // 预分配内存，提升性能
    path.reserve(pointsCount);

    for (int i = 0; i < pointsCount; ++i) {
        // 参数t：从0到2π，控制8字形的完整周期
        double t = 2 * M_PI * i / pointsCount;

        // 伯努利双纽线 (Lemniscate of Bernoulli) 的简化参数方程
        // 标准8字形方程：
        // x = r * cos(t)
        // y = r * sin(t) * cos(t)  <-- 这是生成8字形的关键，让Y轴摆动频率加倍
        // 设计原因：sin(t)*cos(t) = sin(2t)/2，使得Y轴变化频率是X轴的两倍
        //           从而形成8字形的交叉轨迹
        // 提升：使用数学公式而非近似，确保轨迹的平滑性和对称性
        double x = center.x() + radius * std::cos(t);
        double y = center.y() + radius * std::sin(t) * std::cos(t);

        path.append(QPointF(x, y));
    }
    return path;
}

QVector<QPointF> TrajectoryGenerator::createPathFromWaypoints(const QVector<QPointF> &waypoints, double stepSize)
{
    QVector<QPointF> path;
    
    // 防御性检查：至少需要2个点才能形成航迹
    // 提升：避免无效输入导致的错误，提升程序健壮性
    if (waypoints.size() < 2)
        return path;

    // 遍历每一段线段 (Point A -> Point B)
    // 设计原因：将多段航迹连接成连续路径
    for (int i = 0; i < waypoints.size() - 1; ++i) {
        QPointF start = waypoints[i];
        QPointF end   = waypoints[i + 1];

        // 计算两点间的向量和距离
        // 使用QVector2D便于进行向量运算
        // 提升：利用Qt的向量运算，代码简洁且高效
        QVector2D vec(end - start);
        double    distance = vec.length();

        // 归一化向量：获取方向单位向量
        // 提升：避免重复计算方向，提升性能
        QVector2D direction = vec.normalized();

        // 线性插值生成中间点：按步长在起点和终点之间插值
        // 算法：当前点 = 起点 + 方向 * 距离
        // 提升：生成连续的航迹点，确保无人机平滑移动
        for (double d = 0; d < distance; d += stepSize) {
            QPointF p = start + (direction.toPointF() * d);
            path.append(p);
        }
    }
    
    // 确保把最后一个点加进去，避免遗漏终点
    // 提升：确保航迹完整性，无人机能到达所有航点
    path.append(waypoints.last());
    return path;
}
