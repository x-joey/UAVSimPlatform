#include "simulationmanager.h"
#include <cmath>

SimulationManager::SimulationManager()
{
    generatorUavs();
}

const std::vector<std::unique_ptr<UavModel>> &SimulationManager::getUavs() const
{
    return m_uavs;
}

void SimulationManager::generatorUavs()
{
    // 按网格方式布局中心点，避免大量航迹重叠
    const int     total   = m_uav_count;
    const int     cols    = static_cast<int>(std::ceil(std::sqrt(total)));
    const int     rows    = (total + cols - 1) / cols;
    const qreal   spacing = 300.0;   // 网格间距
    const QPointF origin(0.0, 0.0);

    for (int i = 0; i < total; ++i) {
        int row = i / cols;
        int col = i % cols;

        int                       id  = 101 + i;
        std::unique_ptr<UavModel> uav = std::make_unique<UavModel>(id, "Phantom-X");

        // 每个无人机的圆心按网格平铺开来
        QPointF center = origin + QPointF(col * spacing, row * spacing);

        QVector<QPointF> path;
        // 偶数索引用 8 字形，奇数索引用圆形，并稍微变化半径，进一步打散
        qreal radius = 50.0 + (i % 5) * 10.0;
        if (i % 3 == 0) {
            path = TrajectoryGenerator::createEightShapePath(center, radius, 60);
        }
        else if (i % 3 == 2) {
            QVector<QPointF> points;
            points.append(QPointF(122, 69));
            points.append(QPointF(602, 619));
            path = TrajectoryGenerator::createPathFromWaypoints(points, 2);
        }
        else {
            path = TrajectoryGenerator::createCirclePath(center, radius, 60);
        }

        uav->setFlightPath(path);
        uav->updatePosition(0);

        // unique_ptr 不能拷贝，只能 move 进容器
        m_uavs.push_back(std::move(uav));
    }
}
