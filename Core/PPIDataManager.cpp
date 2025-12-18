/**
 * @file PPIDataManager.cpp
 * @brief PPI数据管理器类实现文件
 */

#include "PPIDataManager.h"
#include <QDebug>

PPIDataManager::PPIDataManager(QObject *parent)
    : QObject(parent)
// 调用基类构造函数，初始化Qt对象树
// 提升：使用Qt对象树自动管理内存，避免内存泄漏
{}

void PPIDataManager::updateFromUavs(const std::vector<std::unique_ptr<UavModel>> &uavs,
                                    double                                        ppiCenterX,
                                    double                                        ppiCenterY,
                                    double                                        radius,
                                    uint32_t                                      huanJu)
{
    // 遍历所有UAV，转换为PPI目标数据
    // 设计原因：批量处理所有UAV，确保数据同步
    for (const auto &uav : uavs) {
        int id = uav->getId();

        // 转换UAV到Mubiao：使用适配器模式进行数据转换
        // 提升：将转换逻辑封装在适配器中，提升代码可维护性
        QSharedPointer<Mubiao> mubiao = MubiaoAdapter::convertToMubiao(uav.get(), ppiCenterX, ppiCenterY);

        // 防御性检查：如果转换失败，跳过该UAV
        // 提升：避免空指针导致的程序崩溃，提升系统稳定性
        if (!mubiao)
            continue;
            // QPointF ppiPos = MubiaoAdapter::polarToCartesian(mubiao->fangwei,
            //     mubiao->juli,
            //     0.0,   // PPI局部坐标系中心X
            //     0.0,   // PPI局部坐标系中心Y
            //     radius,
            //     huanJu);

        // 计算相对于雷达中心的笛卡尔坐标（物理坐标，米）
        // 存储航迹时保留物理值，绘制阶段再按当前半径/量程投影到像素坐标，避免缩放后轨迹失真。
        double relX = uav->getX() - ppiCenterX;
        double relY = ppiCenterY - uav->getY();   // Y轴取反：Qt向下为正，雷达向上为正
        QPointF trackPoint(relX, relY);

        // 检查目标是否已存在
        if (m_mubiaoHash.contains(id)) {
            // 更新现有目标：使用线程安全的modify方法
            // 设计原因：原子性更新，确保数据一致性
            // 提升：避免数据竞争，支持多线程并发访问
            m_mubiaoHash.modify(id, [&](Mubiao &mb) {
                // 更新目标的基本属性
                mb.fangwei = mubiao->fangwei;
                mb.juli    = mubiao->juli;
                mb.gaodi   = mubiao->gaodi;

                // 更新航迹：添加新的航迹点（物理坐标，米）
                mb.circleppi_hangji.append(trackPoint);

                // 限制航迹长度为20个点
                // LockedVector自动处理：超出长度时自动移除最旧的点
                // 提升：避免内存无限增长，保持系统性能稳定
            });
        }
        else {
            // 新增目标：首次出现的UAV，创建新的目标记录
            // 提升：支持动态添加目标，适应实时仿真场景
            mubiao->circleppi_hangji.append(trackPoint);
            m_mubiaoHash.insert(id, mubiao);
            // 发出信号通知新目标添加
            // 提升：使用信号槽机制，实现松耦合的通知机制
            emit targetAdded(id);
        }
    }

    // 发出数据更新信号，通知所有监听者
    // 提升：统一的通知机制，避免分散的更新逻辑
    emit dataUpdated();
}

void PPIDataManager::setTargetZhongdian(int targetId, bool zhongdian)
{
    // 检查目标是否存在
    // 提升：防御性编程，避免无效操作
    if (m_mubiaoHash.contains(targetId)) {
        // 使用线程安全的modify方法更新目标状态
        // 提升：原子性操作，确保数据一致性
        m_mubiaoHash.modify(targetId, [zhongdian](Mubiao &mb) { mb.zhongdian = zhongdian; });
        // 发出数据更新信号
        emit dataUpdated();
    }
}

void PPIDataManager::clearAllTargets()
{
    // 清空所有目标数据
    // 提升：支持场景重置功能，提升系统灵活性
    m_mubiaoHash.clear();
    // 发出数据更新信号，通知UI清空显示
    emit dataUpdated();
}
