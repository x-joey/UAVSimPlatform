#ifndef TRAJECTORYGENERATOR_H
#define TRAJECTORYGENERATOR_H

#include <QList>
#include <QPointF>
#include <QVector>
// 工具类，负责计算路径点
class TrajectoryGenerator
{
public:
    TrajectoryGenerator();
    /**
     * 静态方法：生成一个圆形航迹
     * center: 圆心坐标
     * radius: 半径
     * pointsCount: 生成多少个点（越密越平滑）
     **/
    static QVector<QPointF> createCirclePath(QPointF center, double radius, int pointsCount);

    /**
     * 静态方法：生成一个 "8字形" 航迹
     * center: 圆心坐标
     * radius: 半径
     * pointsCount: 生成多少个点（越密越平滑）
     **/
    static QVector<QPointF> createEightShapePath(QPointF center, double radius, int pointsCount);

public:
    QList<QPointF> m_path;
};

#endif   // TRAJECTORYGENERATOR_H
