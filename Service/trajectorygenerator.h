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

    // 新增：根据航点列表生成航迹
    // waypoints: 用户点击的关键点
    // speed: 插值密度（每两个点之间插入多少个点，或者步长）
    static QVector<QPointF> createPathFromWaypoints(const QVector<QPointF> &waypoints, double stepSize = 2.0);

public:
    QList<QPointF> m_path;
};

#endif   // TRAJECTORYGENERATOR_H
