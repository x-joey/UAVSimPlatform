/**
 * @file MubiaoAdapter.h
 * @brief UAV模型到Mubiao的适配器类头文件
 * @details 负责坐标系转换和数据映射，实现UAV模型数据到PPI显示格式的转换
 *          采用适配器模式，解耦UAV模型和PPI显示系统
 */

#ifndef MUBIAOADAPTER_H
#define MUBIAOADAPTER_H

#include "UavModel.h"
#include "Mubiao.h"
#include <QSharedPointer>
#include <QPointF>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @class MubiaoAdapter
 * @brief UAV模型到Mubiao的适配器类
 * @details 负责坐标系转换和数据映射，实现UAV模型数据到PPI显示格式的转换
 *          设计原因：
 *          1. 适配器模式：解耦UAV模型和PPI显示系统，提升代码可维护性
 *          2. 坐标转换：统一处理笛卡尔坐标和极坐标之间的转换
 *          3. 静态方法：无需实例化即可使用，提升使用便利性
 *          4. 数学精确性：使用标准数学公式，确保坐标转换的准确性
 *          提升：
 *          - 代码复用：集中管理坐标转换逻辑，避免重复实现
 *          - 可维护性：修改转换规则只需改一处
 *          - 可测试性：静态方法易于单元测试
 */
class MubiaoAdapter
{
public:
    /**
     * @brief 将UavModel转换为Mubiao（PPI显示用）
     * @param uav 无人机模型指针（常量，不修改原对象）
     * @param ppiCenterX PPI中心X坐标（场景坐标系）
     * @param ppiCenterY PPI中心Y坐标（场景坐标系）
     * @return Mubiao智能指针，如果转换失败返回nullptr
     * @details 将UAV模型的笛卡尔坐标转换为PPI的极坐标格式
     *          转换过程：
     *          1. 计算相对于PPI中心的坐标
     *          2. 转换为极坐标（方位角、距离）
     *          3. 创建Mubiao对象并填充数据
     *          提升：统一的数据转换接口，确保数据格式一致性
     */
    static QSharedPointer<Mubiao> convertToMubiao(
        const UavModel* uav,
        double ppiCenterX,
        double ppiCenterY
    );

    /**
     * @brief 计算笛卡尔坐标到极坐标的转换
     * @param x X坐标（相对于PPI中心）
     * @param y Y坐标（相对于PPI中心）
     * @param[out] outFangWei 输出：方位角（密位，0-6000）
     * @param[out] outJuLi 输出：距离（米）
     * @details 将笛卡尔坐标转换为极坐标
     *          数学公式：
     *          - 距离：r = sqrt(x² + y²)
     *          - 方位角：θ = atan2(x, y) * 3000 / π（转换为密位）
     *          注意：Y轴反向，因为Qt坐标系向下为正，而极坐标向上为正
     *          提升：使用标准数学公式，确保转换精度和正确性
     */
    static void cartesianToPolar(
        double x, double y,
        float& outFangWei,
        uint32_t& outJuLi
    );

    /**
     * @brief 计算极坐标到笛卡尔坐标的转换
     * @param fangwei 方位角（密位，0-6000）
     * @param juli 距离（米）
     * @param ppiCenterX PPI中心X坐标（未使用，保留以兼容接口）
     * @param ppiCenterY PPI中心Y坐标（未使用，保留以兼容接口）
     * @param radius PPI显示半径（像素）
     * @param huanJu 距离环距离（米，如5000表示5km）
     * @return 笛卡尔坐标（相对于PPI中心的本地坐标）
     * @details 将极坐标转换为笛卡尔坐标，并考虑显示缩放
     *          转换过程：
     *          1. 将密位转换为弧度
     *          2. 限制显示距离（最大5倍距离环）
     *          3. 计算显示半径（考虑缩放比例）
     *          4. 转换为笛卡尔坐标
     *          数学公式：
     *          - x = r * sin(θ)
     *          - y = -r * cos(θ)（Y轴向上为负）
     *          提升：统一的坐标转换接口，支持灵活的显示缩放
     */
    static QPointF polarToCartesian(
        float fangwei,
        uint32_t juli,
        double ppiCenterX,
        double ppiCenterY,
        double radius,
        uint32_t huanJu = 5000
    );
};

#endif // MUBIAOADAPTER_H
