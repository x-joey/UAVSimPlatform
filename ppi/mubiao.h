#ifndef MUBIAO_H
#define MUBIAO_H

#include <QObject>
#include <QPainter>
#include <QPointF>
#include <QVector>
#include <QSharedPointer>
#include "LockedHash.h"
#include "LockedVector.h"
// 目标数据结构
struct Mubiao
{
    int pihao = 0;                          // 批号
    float fangwei = 0;                      // 方位角
    float gaodi = 0;                        // 高低角
    uint32_t juli = 0;                      // 距离
    bool zhongdian = false;                 // 是否重点关注
    bool daji_flag = false;                 // 是否已打击
    bool BiaoPai_move = false;              // 标牌是否移动
    QPoint point_biaopai;                   // 标牌位置

    // 关联批号
    int ShangJiPiHao = 0;
    int BenCheGenLeiPiHao = 0;
    int BenCheGenLeiTanCePiHao = 0;
    int BenCheSouLeiPiHao = 0;

    // 航迹数据
    LockedVector<QPointF> circleppi_hangji; // 圆形PPI航迹

    int new_time = 0;                       // 更新时间

    Mubiao() : circleppi_hangji(20) {}      // 航迹缓存20个点

    void draw_mubiao(QPainter* painter, const QPointF& pos)
    {
        if (!painter) return;

        painter->save();

        // 根据目标状态设置颜色
        QColor color = daji_flag ? Qt::red : (zhongdian ? Qt::yellow : Qt::green);
        QPen pen(color, 2);
        painter->setPen(pen);

        // 绘制目标符号（三角形）
        QPolygonF triangle;
        triangle << pos << QPointF(pos.x() - 5, pos.y() + 8)
                 << QPointF(pos.x() + 5, pos.y() + 8);
        painter->drawPolygon(triangle);

        // 绘制批号文本
        painter->setPen(Qt::white);
        painter->drawText(pos.x() + 8, pos.y() - 5, QString::number(pihao));

        painter->restore();
    }

    void qingchu_weiji_circleppi()
    {
        // 清除航迹数据
        circleppi_hangji.clear();
    }
};

// 禁射区域结构
struct JinSheQuYu
{
    double ZuoBianJie = 0;  // 左边界
    double YouBianJie = 0;  // 右边界
};

// 责任扇区结构
struct ZeRenShanQu
{
    int sq_start = 0;  // 起始角度
    int sq_end = 0;    // 结束角度
    int sq_shang = 0;  // 上边界
    int sq_xia = 0;    // 下边界
};

#endif // MUBIAO_H
