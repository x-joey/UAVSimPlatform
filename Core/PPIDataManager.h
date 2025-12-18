/**
 * @file PPIDataManager.h
 * @brief PPI数据管理器类头文件
 * @details 负责将UAV数据转换为PPI显示所需的目标数据，并管理目标航迹的更新
 *          采用适配器模式，将UAV模型数据转换为PPI显示格式
 */

#ifndef PPIDATAMANAGER_H
#define PPIDATAMANAGER_H

#include "UavModel.h"
#include "Mubiao.h"
#include "LockedHash.h"
#include "LockedVector.h"
#include "MubiaoAdapter.h"
#include <QObject>
#include <QPointF>
#include <memory>
#include <vector>

/**
 * @class PPIDataManager
 * @brief PPI数据管理器类
 * @details 负责将UAV数据转换为PPI显示所需的目标数据，并管理目标航迹的更新
 *          设计原因：
 *          1. 数据转换层：将UAV模型数据转换为PPI显示格式，实现数据与显示的分离
 *          2. 线程安全：使用LockedHash确保多线程环境下的数据安全
 *          3. 信号通知：通过Qt信号机制通知数据更新，实现松耦合
 *          4. 航迹管理：自动管理目标航迹，限制长度避免内存无限增长
 *          提升：
 *          - 可维护性：集中管理数据转换逻辑，便于维护和修改
 *          - 性能：使用线程安全容器，支持多线程并发访问
 *          - 扩展性：易于添加新的数据转换规则和显示属性
 */
class PPIDataManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针，用于Qt对象树管理
     * @details 初始化数据管理器，创建线程安全的数据容器
     *          提升：使用Qt对象树自动管理内存，避免内存泄漏
     */
    explicit PPIDataManager(QObject* parent = nullptr);

    /**
     * @brief 从UAV列表更新目标数据
     * @param uavs UAV模型列表的常量引用
     * @param ppiCenterX PPI中心X坐标（场景坐标系）
     * @param ppiCenterY PPI中心Y坐标（场景坐标系）
     * @param radius PPI显示半径（像素）
     * @param huanJu 距离环距离（米，默认5000米）
     * @details 将UAV模型数据转换为PPI目标数据，并更新航迹
     *          算法：
     *          1. 遍历所有UAV，转换为Mubiao格式
     *          2. 计算目标在PPI上的显示位置
     *          3. 更新或新增目标数据
     *          4. 管理航迹长度，限制为20个点
     *          提升：自动同步UAV数据到PPI显示，保持数据一致性
     */
    void updateFromUavs(
        const std::vector<std::unique_ptr<UavModel>>& uavs,
        double ppiCenterX,
        double ppiCenterY,
        double radius,
        uint32_t huanJu = 5000
    );

    /**
     * @brief 获取目标数据哈希表
     * @return 目标数据哈希表的引用
     * @details 返回引用而非拷贝，避免不必要的内存复制
     *          提升：支持外部直接访问数据，提升性能
     *          注意：返回的是线程安全的容器，可安全地在多线程环境中使用
     */
    LockedHash<Mubiao>& getMubiaoHash() { return m_mubiaoHash; }

    /**
     * @brief 设置目标为重点关注
     * @param targetId 目标ID
     * @param zhongdian 是否为重点关注
     * @details 标记目标为重点关注，影响显示颜色和样式
     *          提升：支持目标优先级管理，便于操作员关注重要目标
     */
    void setTargetZhongdian(int targetId, bool zhongdian);

    /**
     * @brief 清除所有目标数据
     * @details 清空所有目标数据，用于重置或清理场景
     *          提升：支持场景重置功能，提升系统灵活性
     */
    void clearAllTargets();

signals:
    /**
     * @brief 数据更新信号
     * @details 当目标数据发生变化时发出，通知UI更新显示
     *          提升：使用信号槽机制实现松耦合，提升代码可维护性
     */
    void dataUpdated();

    /**
     * @brief 目标添加信号
     * @param targetId 新添加的目标ID
     * @details 当新目标被添加时发出，可用于日志记录或UI更新
     */
    void targetAdded(int targetId);

    /**
     * @brief 目标移除信号
     * @param targetId 被移除的目标ID
     * @details 当目标被移除时发出，可用于清理相关资源
     */
    void targetRemoved(int targetId);

private:
    /**
     * @brief 存储目标数据的线程安全哈希表
     * @details 使用LockedHash确保多线程环境下的数据安全
     *          键为目标ID，值为目标数据（Mubiao）的智能指针
     *          提升：支持多线程并发访问，提升系统性能和稳定性
     */
    LockedHash<Mubiao> m_mubiaoHash;
};

#endif // PPIDATAMANAGER_H
