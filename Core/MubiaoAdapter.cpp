/**
 * @file MubiaoAdapter.cpp
 * @brief UAV模型到Mubiao的适配器类实现文件
 */

#include "MubiaoAdapter.h"
#include <cmath>

QSharedPointer<Mubiao> MubiaoAdapter::convertToMubiao(
    const UavModel* uav,
    double ppiCenterX,
    double ppiCenterY)
{
    // 防御性检查：避免空指针导致的崩溃
    // 提升：提升系统稳定性，避免程序异常终止
    if (!uav) return nullptr;

    // 创建Mubiao对象，使用智能指针管理内存
    // 提升：自动管理内存，避免内存泄漏
    auto mubiao = QSharedPointer<Mubiao>::create();

    // 复制基本属性：ID直接映射
    mubiao->pihao = uav->getId();

    // 计算相对于PPI中心的坐标
    // 设计原因：PPI显示需要相对于中心点的坐标
    // 提升：统一坐标系，简化后续计算
    double relX = uav->getX() - ppiCenterX;
    // Y轴反向：Qt坐标系向下为正，而极坐标向上为正
    // 提升：确保坐标转换的正确性
    double relY = ppiCenterY - uav->getY();

    // 转换为极坐标：调用静态方法进行坐标转换
    // 提升：复用转换逻辑，避免代码重复
    cartesianToPolar(relX, relY, mubiao->fangwei, mubiao->juli);

    // 初始化默认属性
    mubiao->gaodi = 0; // UAV暂时不考虑高度，未来可扩展
    mubiao->zhongdian = false; // 默认非重点关注
    mubiao->daji_flag = false; // 默认未打击

    return mubiao;
}

void MubiaoAdapter::cartesianToPolar(
    double x, double y,
    float& outFangWei,
    uint32_t& outJuLi)
{
    // 计算距离：使用欧几里得距离公式
    // 数学公式：r = sqrt(x² + y²)
    // 提升：使用标准数学公式，确保计算精度
    outJuLi = static_cast<uint32_t>(std::sqrt(x * x + y * y));

    // 计算方位角（北向为0，顺时针）
    // 注意：使用atan2(x, y)而非atan2(y, x)，因为北向是Y轴正方向
    // 数学原理：atan2(y, x)返回从x轴正方向到点(x,y)的角度
    //           但我们需要从Y轴正方向（北向）的角度，所以使用atan2(x, y)
    // 提升：确保方位角的正确性，符合雷达显示习惯
    double angle = std::atan2(x, y);
    
    // 将角度规范化到[0, 2π)范围
    // 提升：确保角度值在有效范围内，避免负角度
    if (angle < 0) angle += 2 * M_PI;

    // 转换为密位（0-6000）
    // 转换公式：360度 = 6000密位，1密位 = 0.06度 = π/3000 弧度
    // 因此：密位 = 弧度 * 3000 / π
    // 提升：使用标准密位制，符合军事雷达显示习惯
    outFangWei = static_cast<float>(angle * 3000.0 / M_PI);
}

QPointF MubiaoAdapter::polarToCartesian(
    float fangwei,
    uint32_t juli,
    double ppiCenterX,
    double ppiCenterY,
    double radius,
    uint32_t huanJu)
{
    // 未使用的参数：保留以兼容接口
    // 注意：PPI图元的坐标系原点在中心，所以不需要使用这些参数
    Q_UNUSED(ppiCenterX)
    Q_UNUSED(ppiCenterY)

    // 限制绘图距离：最大显示5倍距离环
    // 设计原因：避免过远目标占用过多显示空间
    // 提升：优化显示效果，聚焦有效范围
    uint16_t huitu_juli = static_cast<uint16_t>(juli);
    if (huitu_juli > (5 * huanJu))
    {
        huitu_juli = static_cast<uint16_t>(5 * huanJu);
    }

    // 密位转弧度：将密位制转换为弧度制
    // 转换公式：弧度 = 密位 * π / 3000
    // 提升：统一使用弧度制进行数学计算，提升精度
    double angle = fangwei * M_PI / 3000.0;

    // 计算在PPI上的显示半径（考虑缩放）
    // 缩放公式：显示半径 = 实际距离 * 显示半径 * 0.2 / 单环距离
    // 设计原因：将实际距离映射到显示半径，实现距离缩放
    // 0.2是缩放因子，确保显示比例合理
    // 提升：支持灵活的显示缩放，适应不同显示需求
    double displayRadius = huitu_juli * radius * 0.2 / huanJu;

    // 极坐标转笛卡尔坐标（相对于PPI中心的本地坐标）
    // 数学公式：
    // x = r * sin(θ)
    // y = -r * cos(θ)（Y轴向上为负，因为Qt坐标系向下为正）
    // 注意：PPIGraphicsItem的painter原点就是PPI中心，所以直接返回相对坐标
    // 提升：使用标准数学公式，确保坐标转换的正确性
    double x = displayRadius * std::sin(angle);
    double y = -displayRadius * std::cos(angle); // Y轴向上为负

    return QPointF(x, y);
}
