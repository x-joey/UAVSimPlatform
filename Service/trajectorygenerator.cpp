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
    // 避免 vector 在添加元素时频繁扩容
    path.reserve(pointsCount);

    for (int i = 0; i < pointsCount; ++i) {
        // 计算当前角度 (0 到 2*PI)
        double angle = 2 * M_PI * i / pointsCount;

        // 数学公式:
        // x = center_x + r * cos(θ)
        // y = center_y + r * sin(θ)
        double x = center.x() + radius * std::cos(angle);
        double y = center.y() + radius * std::sin(angle);

        path.append(QPointF(x, y));
    }

    return path;
}

QVector<QPointF> TrajectoryGenerator::createEightShapePath(QPointF center, double radius, int pointsCount)
{
    QVector<QPointF> path;
    path.reserve(pointsCount);

    for (int i = 0; i < pointsCount; ++i) {
        double t = 2 * M_PI * i / pointsCount;

        // 伯努利双纽线 (Lemniscate of Bernoulli) 的简化参数方程
        // x = r * cos(t)
        // y = r * sin(t) * cos(t)  <-- 这是一个简单的生成8字形的方法
        double x = center.x() + radius * std::cos(t);
        double y = center.y() + radius * std::sin(t) * std::cos(t);   // 让Y轴摆动频率加倍

        path.append(QPointF(x, y));
    }
    return path;
}

QVector<QPointF> TrajectoryGenerator::createPathFromWaypoints(const QVector<QPointF> &waypoints, double stepSize)
{
    QVector<QPointF> path;
    if (waypoints.size() < 2)
        return path;

    // 遍历每一段线段 (Point A -> Point B)
    for (int i = 0; i < waypoints.size() - 1; ++i) {
        QPointF start = waypoints[i];
        QPointF end   = waypoints[i + 1];

        // 计算两点距离
        QVector2D vec(end - start);
        double    distance = vec.length();

        // 归一化向量 (方向)
        QVector2D direction = vec.normalized();

        // 线性插值生成中间点
        for (double d = 0; d < distance; d += stepSize) {
            // 当前点 = 起点 + 方向 * 距离
            QPointF p = start + (direction.toPointF() * d);
            path.append(p);
        }
    }
    // 确保把最后一个点加进去
    path.append(waypoints.last());
    return path;
}
