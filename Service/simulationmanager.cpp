/**
 * @file simulationmanager.cpp
 * @brief 仿真管理器类实现文件
 */

#include "simulationmanager.h"
#include <cmath>

SimulationManager::SimulationManager()
{
    // 构造函数中自动生成无人机，确保对象立即可用
    // 提升：简化使用流程，避免忘记初始化的错误
    generatorUavs();
}

const std::vector<std::unique_ptr<UavModel>> &SimulationManager::getUavs() const
{
    // 返回常量引用，避免不必要的拷贝
    // 提升：减少内存占用和性能开销，特别是当无人机数量较多时
    return m_uavs;
}

void SimulationManager::generatorUavs()
{
    // 网格布局策略：按网格方式布局中心点，避免大量航迹重叠
    // 设计原因：均匀分布无人机，提升可视化效果和性能
    const int     total   = m_uav_count;
    // 计算网格列数：使用平方根向上取整，确保接近正方形布局
    // 提升：接近正方形的网格布局视觉效果最佳
    const int     cols    = static_cast<int>(std::ceil(std::sqrt(total)));
    // 计算网格行数：向上取整确保所有无人机都能放置
    const int     rows    = (total + cols - 1) / cols;
    // 网格间距：扩大到 800 像素，确保不同无人机的航迹不会重叠
    // 提升：避免视觉混乱，便于观察每个无人机的运动轨迹
    const qreal   spacing = 800.0;
    // 网格原点：从场景左上方开始布局，避免都堆积在原点
    const QPointF origin(-2000.0, -2000.0);

    // 遍历生成每架无人机
    for (int i = 0; i < total; ++i) {
        // 计算当前无人机在网格中的位置
        int row = i / cols;
        int col = i % cols;

        // 生成唯一ID：从101开始，避免与系统保留ID冲突
        int                       id  = 101 + i;
        // 使用智能指针创建无人机实例，自动管理内存
        // 提升：避免内存泄漏，符合现代C++最佳实践
        std::unique_ptr<UavModel> uav = std::make_unique<UavModel>(id, "Phantom-X");

        // 计算无人机航迹中心点：每个无人机的圆心按网格平铺开来
        // 提升：均匀分布，避免重叠，提升可视化效果
        QPointF center = origin + QPointF(col * spacing, row * spacing);

        QVector<QPointF> path;
        // 动态调整航迹半径：根据索引变化，进一步打散无人机位置
        // 提升：增加视觉多样性，避免所有无人机使用相同大小的航迹
        qreal radius = 100.0 + (i % 5) * 30.0;

        // 根据索引分配不同的航迹模式，增加多样性
        // 设计原因：不同航迹模式展示不同的运动特性，提升仿真真实性
        if (i % 3 == 0) {
            // 8字形航迹：展示复杂运动模式
            path = TrajectoryGenerator::createEightShapePath(center, radius, 60);
        }
        else if (i % 3 == 2) {
            // 航点航迹：展示直线运动模式，使用相对于中心点的坐标
            QVector<QPointF> points;
            points.append(center + QPointF(-radius, -radius));
            points.append(center + QPointF(radius, radius));
            points.append(center + QPointF(-radius, radius));
            points.append(center + QPointF(radius, -radius));
            path = TrajectoryGenerator::createPathFromWaypoints(points, 5.0);
        }
        else {
            // 圆形航迹：展示标准圆周运动
            path = TrajectoryGenerator::createCirclePath(center, radius, 60);
        }

        // 设置航迹并初始化位置
        uav->setFlightPath(path);
        uav->updatePosition(0);

        // unique_ptr 不能拷贝，只能 move 进容器
        // 提升：转移所有权而非拷贝，避免不必要的对象复制，提升性能
        m_uavs.push_back(std::move(uav));
    }
}
