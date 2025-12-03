#ifndef FLIGHTPATH_H
#define FLIGHTPATH_H

#include <QList>
#include <QPointF>

class FlightPath
{
public:
    FlightPath();
    // 生成匀速直线运动航迹

private:
    QList<QPointF> m_path;
};

#endif // FLIGHTPATH_H
