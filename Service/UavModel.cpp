/**
 * @file UavModel.cpp
 * @brief 无人机模型类实现文件
 */

#include "UavModel.h"
#include <QDebug>   // 用于打印日志

UavModel::UavModel(int id, QString name)
    : m_id(id)
    , m_name(name)
    , m_x(0.0)
    , m_y(0.0)   // 初始化列表，C++ 特有且推荐的写法
    // 提升：使用初始化列表而非构造函数体内赋值，避免先默认构造再赋值
    //       对于基本类型性能提升不明显，但对于复杂类型可避免不必要的构造和析构
{}

void UavModel::updatePosition(int stepIndex)
{
    // 检查航迹是否已设置
    // 设计原因：防御性编程，避免空指针或未初始化导致的崩溃
    if (m_path.isEmpty()) {
        qDebug() << "Warning: No flight path set!";
        return;
    }

    // 防止数组越界：使用模运算确保索引始终在有效范围内
    // C++ 中越界会导致未定义行为或程序直接崩溃，比 Java 更严重
    // 提升：确保程序稳定性，避免崩溃和数据损坏
    int safeIndex = stepIndex % m_path.size();

    // 从航迹中获取目标位置并更新坐标
    QPointF nextPos = m_path[safeIndex];
    m_x             = nextPos.x();
    m_y             = nextPos.y();

    // 调试输出：记录位置更新信息
    // 提升：便于调试和跟踪无人机运动轨迹
    qDebug() << "UAV moved to index" << safeIndex << ":" << m_x << m_y;
}

void UavModel::setFlightPath(const QVector<QPointF> &path)
{
    // 使用拷贝赋值，确保航迹数据独立
    // 提升：避免外部修改影响内部数据，提升数据安全性
    m_path = path;
}

void UavModel::setCurrentPoint(int currentPoint)
{
    // 直接赋值，支持外部精确控制
    // 提升：支持回放、跳转等高级功能
    m_currentPoint = currentPoint;
}

int UavModel::getId() const
{
    // 返回常量值，确保ID不可修改
    return m_id;
}

int UavModel::getCurrentPoint() const
{
    return m_currentPoint;
}

double UavModel::getX() const
{
    return m_x;
}

double UavModel::getY() const
{
    return m_y;
}

QString UavModel::getName() const
{
    return m_name;
}

const QVector<QPointF> &UavModel::getPath() const
{
    // 返回常量引用，避免不必要的拷贝
    // 提升：当航迹点数量较多时，避免大量内存复制，提升性能
    return m_path;
}
