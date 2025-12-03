#include "trajectorygenerator.h"
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
