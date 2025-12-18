/**
 * @file CoordinateConverter.h
 * @brief 坐标转换系统
 * @details 实现地理坐标（WGS84/GCJ-02）与场景坐标之间的转换
 *          支持中国火星坐标系（GCJ-02）的纠偏
 */

#ifndef COORDINATECONVERTER_H
#define COORDINATECONVERTER_H

#include <QPointF>
#include <QGeoCoordinate>
#include <cmath>

/**
 * @class CoordinateConverter
 * @brief 坐标转换工具类
 * @details 提供地理坐标系与场景坐标系之间的双向转换
 *          - 地理坐标：WGS84经纬度（度）
 *          - 场景坐标：以中心点为原点的米制坐标
 */
class CoordinateConverter
{
public:
    /**
     * @brief 设置参考中心点（地理坐标）
     * @param centerLat 中心点纬度（度）
     * @param centerLon 中心点经度（度）
     * @details 所有场景坐标以此点为原点(0, 0)
     */
    static void setReferenceCenter(double centerLat, double centerLon);

    /**
     * @brief 地理坐标转场景坐标
     * @param latitude 纬度（度）
     * @param longitude 经度（度）
     * @return 场景坐标（米），原点为参考中心点
     */
    static QPointF geoToScene(double latitude, double longitude);

    /**
     * @brief 场景坐标转地理坐标
     * @param scenePos 场景坐标（米）
     * @return 地理坐标（QGeoCoordinate）
     */
    static QGeoCoordinate sceneToGeo(const QPointF &scenePos);

    /**
     * @brief WGS84坐标转GCJ-02（火星坐标系）
     * @param wgsLat WGS84纬度
     * @param wgsLon WGS84经度
     * @param gcjLat 输出：GCJ-02纬度
     * @param gcjLon 输出：GCJ-02经度
     * @details 用于中国地图数据的坐标纠偏
     */
    static void wgs84ToGcj02(double wgsLat, double wgsLon, double &gcjLat, double &gcjLon);

    /**
     * @brief GCJ-02转WGS84坐标
     * @param gcjLat GCJ-02纬度
     * @param gcjLon GCJ-02经度
     * @param wgsLat 输出：WGS84纬度
     * @param wgsLon 输出：WGS84经度
     */
    static void gcj02ToWgs84(double gcjLat, double gcjLon, double &wgsLat, double &wgsLon);

    /**
     * @brief 计算两个地理坐标点之间的距离（米）
     * @param lat1 点1纬度
     * @param lon1 点1经度
     * @param lat2 点2纬度
     * @param lon2 点2经度
     * @return 距离（米）
     */
    static double distanceBetween(double lat1, double lon1, double lat2, double lon2);

private:
    static double s_centerLat;   // 参考中心点纬度
    static double s_centerLon;   // 参考中心点经度

    // 地球半径常量
    static constexpr double EARTH_RADIUS = 6378137.0;  // 米
    static constexpr double PI = 3.14159265358979323846;

    // 坐标纠偏相关常数
    static constexpr double A = 6378245.0;           // 长半轴
    static constexpr double EE = 0.00669342162296594323;  // 偏心率平方

    // 判断坐标是否在中国境内
    static bool isInChina(double lat, double lon);

    // 坐标偏移计算
    static double transformLat(double x, double y);
    static double transformLon(double x, double y);
};

#endif // COORDINATECONVERTER_H
