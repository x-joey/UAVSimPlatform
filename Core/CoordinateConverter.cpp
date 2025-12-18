/**
 * @file CoordinateConverter.cpp
 * @brief 坐标转换系统实现
 */

#include "CoordinateConverter.h"
#include <cmath>

// 静态成员初始化（天安门坐标）
double CoordinateConverter::s_centerLat = 39.9042;   // 天安门纬度
double CoordinateConverter::s_centerLon = 116.4074;  // 天安门经度

void CoordinateConverter::setReferenceCenter(double centerLat, double centerLon)
{
    s_centerLat = centerLat;
    s_centerLon = centerLon;
}

QPointF CoordinateConverter::geoToScene(double latitude, double longitude)
{
    // 纬度差转换为y坐标（米）
    // 1度纬度 ≈ 111320米
    double deltaLat = latitude - s_centerLat;
    double y = deltaLat * 111320.0;

    // 经度差转换为x坐标（米）
    // 1度经度 = 111320 * cos(纬度) 米
    double deltaLon = longitude - s_centerLon;
    double x = deltaLon * 111320.0 * std::cos(s_centerLat * PI / 180.0);

    return QPointF(x, -y);  // 注意：y轴向下为正，需要取反
}

QGeoCoordinate CoordinateConverter::sceneToGeo(const QPointF &scenePos)
{
    // x坐标转经度差
    double deltaLon = scenePos.x() / (111320.0 * std::cos(s_centerLat * PI / 180.0));
    double longitude = s_centerLon + deltaLon;

    // y坐标转纬度差（注意y轴反向）
    double deltaLat = -scenePos.y() / 111320.0;
    double latitude = s_centerLat + deltaLat;

    return QGeoCoordinate(latitude, longitude);
}

double CoordinateConverter::distanceBetween(double lat1, double lon1, double lat2, double lon2)
{
    // Haversine公式计算球面距离
    double dLat = (lat2 - lat1) * PI / 180.0;
    double dLon = (lon2 - lon1) * PI / 180.0;

    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(lat1 * PI / 180.0) * std::cos(lat2 * PI / 180.0) *
               std::sin(dLon / 2) * std::sin(dLon / 2);

    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    return EARTH_RADIUS * c;
}

bool CoordinateConverter::isInChina(double lat, double lon)
{
    // 粗略判断坐标是否在中国境内
    return (lon >= 72.004 && lon <= 137.8347) &&
           (lat >= 0.8293 && lat <= 55.8271);
}

double CoordinateConverter::transformLat(double x, double y)
{
    double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * std::sqrt(std::abs(x));
    ret += (20.0 * std::sin(6.0 * x * PI) + 20.0 * std::sin(2.0 * x * PI)) * 2.0 / 3.0;
    ret += (20.0 * std::sin(y * PI) + 40.0 * std::sin(y / 3.0 * PI)) * 2.0 / 3.0;
    ret += (160.0 * std::sin(y / 12.0 * PI) + 320 * std::sin(y * PI / 30.0)) * 2.0 / 3.0;
    return ret;
}

double CoordinateConverter::transformLon(double x, double y)
{
    double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * std::sqrt(std::abs(x));
    ret += (20.0 * std::sin(6.0 * x * PI) + 20.0 * std::sin(2.0 * x * PI)) * 2.0 / 3.0;
    ret += (20.0 * std::sin(x * PI) + 40.0 * std::sin(x / 3.0 * PI)) * 2.0 / 3.0;
    ret += (150.0 * std::sin(x / 12.0 * PI) + 300.0 * std::sin(x / 30.0 * PI)) * 2.0 / 3.0;
    return ret;
}

void CoordinateConverter::wgs84ToGcj02(double wgsLat, double wgsLon, double &gcjLat, double &gcjLon)
{
    if (!isInChina(wgsLat, wgsLon)) {
        // 不在中国境内，不需要转换
        gcjLat = wgsLat;
        gcjLon = wgsLon;
        return;
    }

    double dLat = transformLat(wgsLon - 105.0, wgsLat - 35.0);
    double dLon = transformLon(wgsLon - 105.0, wgsLat - 35.0);

    double radLat = wgsLat / 180.0 * PI;
    double magic = std::sin(radLat);
    magic = 1 - EE * magic * magic;
    double sqrtMagic = std::sqrt(magic);

    dLat = (dLat * 180.0) / ((A * (1 - EE)) / (magic * sqrtMagic) * PI);
    dLon = (dLon * 180.0) / (A / sqrtMagic * std::cos(radLat) * PI);

    gcjLat = wgsLat + dLat;
    gcjLon = wgsLon + dLon;
}

void CoordinateConverter::gcj02ToWgs84(double gcjLat, double gcjLon, double &wgsLat, double &wgsLon)
{
    if (!isInChina(gcjLat, gcjLon)) {
        wgsLat = gcjLat;
        wgsLon = gcjLon;
        return;
    }

    // 迭代逼近
    double tempLat, tempLon;
    wgs84ToGcj02(gcjLat, gcjLon, tempLat, tempLon);

    wgsLat = gcjLat - (tempLat - gcjLat);
    wgsLon = gcjLon - (tempLon - gcjLon);
}
