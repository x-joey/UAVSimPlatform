#ifndef SIMULATIONMANAGER_H
#define SIMULATIONMANAGER_H
#include "UavModel.h"
#include "trajectorygenerator.h"
#include <memory>
#include <vector>
class SimulationManager
{
public:
    SimulationManager();
    const std::vector<std::unique_ptr<UavModel>> &getUavs() const;
    // 生成并保存 UAV 列表，只修改成员，不返回拷贝
    void generatorUavs();

private:
    std::vector<std::unique_ptr<UavModel>> m_uavs;
    int                                    m_uav_count = 5;
};

#endif   // SIMULATIONMANAGER_H
