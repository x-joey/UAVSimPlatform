#include "UavModel.h"
#include <QDebug>   // 用于打印日志

UavModel::UavModel(int id, QString name)
    : m_id(id)
    , m_name(name)
    , m_x(0.0)
    , m_y(0.0)   // 初始化列表，C++ 特有且推荐的写法
{}

void UavModel::updatePosition(int stepIndex)
{
    // 模拟一些简单的移动逻辑
    //    m_x += 1.5;
    //    m_y += 0.5;
    //    qDebug() << "Core Logic: UAV" << m_id << "moved to (" << m_x << "," << m_y << ")";

    if (m_path.isEmpty()) {
        qDebug() << "Warning: No flight path set!";
        return;
    }

    // 防止数组越界 (C++ 中越界会导致程序直接崩溃，比 Java 严重)
    int safeIndex = stepIndex % m_path.size();

    QPointF nextPos = m_path[safeIndex];
    m_x             = nextPos.x();
    m_y             = nextPos.y();

    qDebug() << "UAV moved to index" << safeIndex << ":" << m_x << m_y;
}

void UavModel::setFlightPath(const QVector<QPointF> &path)
{
    m_path = path;
}

int UavModel::getId() const
{
    return m_id;
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
    return m_path;
}
