/**
 * @file simulationmanager.h
 * @brief 仿真管理器类头文件
 * @details 负责管理整个仿真系统中的所有无人机实例，包括生成、布局和生命周期管理
 *          采用网格布局策略避免航迹重叠，提升可视化效果和性能
 */

#ifndef SIMULATIONMANAGER_H
#define SIMULATIONMANAGER_H
#include "UavModel.h"
#include "trajectorygenerator.h"
#include <memory>
#include <vector>

/**
 * @class SimulationManager
 * @brief 仿真管理器类
 * @details 负责管理仿真系统中所有无人机的创建、布局和访问
 *          设计原因：
 *          1. 集中管理所有无人机实例，避免分散管理导致的资源泄漏
 *          2. 使用unique_ptr确保内存安全，自动管理对象生命周期
 *          3. 网格布局策略避免大量无人机重叠，提升渲染性能和可视化效果
 *          4. 提供统一的访问接口，便于上层模块获取无人机数据
 */
class SimulationManager
{
public:
    /**
     * @brief 构造函数
     * @details 自动调用generatorUavs()生成初始无人机队列
     *          提升：确保对象构造完成后立即可用，无需额外初始化步骤
     */
    SimulationManager();

    /**
     * @brief 获取所有无人机实例的常量引用
     * @return 无人机向量容器的常量引用
     * @details 返回常量引用而非拷贝，避免不必要的内存复制
     *          提升：减少内存占用和拷贝开销，提升性能
     *          注意：返回的是unique_ptr的引用，外部不能获取所有权
     */
    const std::vector<std::unique_ptr<UavModel>> &getUavs() const;

    /**
     * @brief 生成并初始化无人机列表
     * @details 采用网格布局策略生成无人机，避免航迹重叠
     *          设计原因：
     *          1. 网格布局确保无人机均匀分布，避免视觉混乱
     *          2. 不同无人机使用不同航迹模式（圆形、8字形、航点），增加多样性
     *          3. 动态调整半径，进一步打散无人机位置
     *          提升：提升可视化效果，减少渲染冲突，便于观察和分析
     */
    void generatorUavs();

private:
    /**
     * @brief 存储所有无人机实例的容器
     * @details 使用unique_ptr管理内存，确保自动释放
     *          使用vector而非list，提升缓存局部性和访问性能
     */
    std::vector<std::unique_ptr<UavModel>> m_uavs;

    /**
     * @brief 无人机数量
     * @details 默认值为5，可根据需要调整
     *          设计为成员变量而非硬编码，提升可配置性
     */
    int                                    m_uav_count = 5;
};

#endif   // SIMULATIONMANAGER_H
