/**
 * @file Mubiao.h
 * @brief 目标数据结构头文件
 * @details 定义PPI显示所需的目标数据结构和相关辅助结构
 *          包含目标信息、航迹数据、禁射区域和责任扇区等
 */

#ifndef MUBIAO_H
#define MUBIAO_H

#include "LockedVector.h"
#include <QObject>
#include <QPainter>
#include <QPointF>
#include <QSharedPointer>
#include <QVector>

/**
 * @struct Mubiao
 * @brief 目标数据结构
 * @details 用于PPI显示的目标信息，包含位置、状态、航迹等数据
 *          设计原因：
 *          1. 数据封装：将目标的所有信息集中在一个结构中，便于管理
 *          2. 线程安全：使用LockedVector存储航迹，支持多线程访问
 *          3. 自包含绘制：提供绘制方法，实现数据与显示的紧耦合
 *          4. 内存优化：限制航迹长度，避免内存无限增长
 *          提升：
 *          - 可维护性：清晰的数据结构，易于理解和修改
 *          - 性能：限制航迹长度，保持系统性能稳定
 *          - 扩展性：易于添加新的目标属性
 */
struct Mubiao
{
    /**
     * @brief 批号（目标ID）
     * @details 用于唯一标识目标，通常对应UAV的ID
     */
    int pihao = 0;

    /**
     * @brief 方位角（密位，0-6000）
     * @details 目标相对于雷达的方位角，使用密位制
     *          0-6000密位对应0-360度
     */
    float fangwei = 0;

    /**
     * @brief 高低角
     * @details 目标相对于水平面的仰角，当前未使用
     */
    float gaodi = 0;

    /**
     * @brief 距离（米）
     * @details 目标到雷达的距离，单位米
     */
    uint32_t juli = 0;

    /**
     * @brief 是否重点关注
     * @details 标记目标为重点关注，影响显示颜色和样式
     *          提升：支持目标优先级管理，便于操作员关注重要目标
     */
    bool zhongdian = false;

    /**
     * @brief 是否已打击
     * @details 标记目标是否已被打击，影响显示颜色
     *          提升：支持打击状态管理，便于战场态势显示
     */
    bool daji_flag = false;

    /**
     * @brief 是否正在导引
     * @details 标记目标是否正在被导引，影响显示样式
     *          提升：支持导引状态管理，便于操作员识别导引目标
     */
    bool daoyin_flag = false;

    /**
     * @brief 标牌是否移动
     * @details 标记目标标牌是否可以移动，用于UI交互
     */
    bool BiaoPai_move = false;

    /**
     * @brief 标牌位置
     * @details 目标标牌在屏幕上的显示位置
     */
    QPoint point_biaopai;

    /**
     * @brief 标牌偏移量（相对于目标位置）
     * @details 用于PPI上标牌的拖动，存储标牌相对于目标的偏移
     */
    QPointF labelOffset = QPointF(20, -20);

    /**
     * @brief 圆形PPI航迹数据（物理坐标，单位：米）
     * @details 存储相对于雷达中心的笛卡尔坐标 (x,y)，单位米。
     *          绘制时按当前 PPI 半径 / 量程实时投影到像素坐标，
     *          避免量程或半径变化后轨迹比例失真。
     *          限制最大长度，超出时自动移除最旧的点，防止内存增长。
     */
    LockedVector<QPointF> circleppi_hangji;   // 这里的 QPointF 是物理坐标（米）

    /**
     * @brief 更新时间
     * @details 记录目标数据的最后更新时间，用于数据有效性判断
     */
    int new_time = 0;

    /**
     * @brief 构造函数
     * @details 初始化航迹容器，限制最大长度为20个点
     *          提升：自动限制航迹长度，避免内存无限增长
     */
    Mubiao()
        : circleppi_hangji(200)
    {}

    /**
     * @brief 绘制目标符号
     * @param painter 画笔对象，用于绘制
     * @param pos 目标在PPI上的显示位置
     * @details 根据目标状态绘制不同颜色和样式的符号
     *          绘制逻辑：
     *          1. 根据状态设置颜色和样式：
     *             - 已打击：红色圆形（不绘制）
     *             - 导引中：红色圆形 + 外圈 + 闪烁效果
     *             - 重点关注：黄色圆形 + 连线航迹
     *             - 普通：绿色圆形
     *          2. 绘制目标符号
     *          3. 绘制批号文本
     *          提升：自包含绘制逻辑，简化外部调用代码
     */
    void draw_mubiao(QPainter *painter, const QPointF &pos)
    {
        // 防御性检查：避免空指针
        if (!painter)
            return;

        // 保存画笔状态，确保不影响后续绘制
        painter->save();

        QColor color;
        int circleRadius = 4;  // 目标圆形半径

        // 根据目标状态设置颜色和样式
        if (daji_flag) {
            // 已打击：红色
            color = Qt::red;
        }
        else if (daoyin_flag) {
            // 导引中：红色 + 外圈强调
            color = Qt::red;
            circleRadius = 5;

            // 绘制外圈（闪烁效果）
            QPen outerPen(QColor(255, 0, 0, 150), 2);
            painter->setPen(outerPen);
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(pos, 8, 8);
        }
        else if (zhongdian) {
            // 重点关注：黄色
            color = Qt::yellow;
            circleRadius = 4;
        }
        else {
            // 普通：绿色
            color = Qt::green;
            circleRadius = 3;
        }

        // 绘制目标圆形
        painter->setBrush(QBrush(color, Qt::SolidPattern));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(pos, circleRadius, circleRadius);

        // 不再在这里绘制批号文本，由PPIGraphicsItem绘制标牌
        // 标牌会在 paintTargets 中统一绘制

        // 恢复画笔状态
        painter->restore();
    }

    /**
     * @brief 清除航迹数据
     * @details 清空所有航迹点，用于重置或清理
     *          提升：支持航迹重置功能，提升系统灵活性
     */
    void qingchu_weiji_circleppi() { circleppi_hangji.clear(); }
};

/**
 * @struct JinSheQuYu
 * @brief 禁射区域结构
 * @details 定义禁射区域的边界角度
 *          设计原因：用于标记不能射击的区域，提升安全性
 *          提升：支持安全区域管理，避免误伤
 */
struct JinSheQuYu
{
    /**
     * @brief 左边界（度）
     * @details 禁射区域的起始角度，单位度
     */
    double ZuoBianJie = 0;

    /**
     * @brief 右边界（度）
     * @details 禁射区域的结束角度，单位度
     */
    double YouBianJie = 0;
};

/**
 * @struct ZeRenShanQu
 * @brief 责任扇区结构
 * @details 定义责任扇区的边界信息
 *          设计原因：用于标记责任区域，便于任务分配
 *          提升：支持区域管理，提升任务组织效率
 */
struct ZeRenShanQu
{
    /**
     * @brief 起始角度（密位）
     * @details 责任扇区的起始方位角，单位密位
     */
    int sq_start = 0;

    /**
     * @brief 结束角度（密位）
     * @details 责任扇区的结束方位角，单位密位
     */
    int sq_end = 0;

    /**
     * @brief 上边界
     * @details 责任扇区的上边界（高度或距离）
     */
    int sq_shang = 0;

    /**
     * @brief 下边界
     * @details 责任扇区的下边界（高度或距离）
     */
    int sq_xia = 0;
};

#endif   // MUBIAO_H
