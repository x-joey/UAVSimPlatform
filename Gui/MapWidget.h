/**
 * @file MapWidget.h
 * @brief Qt Location地图组件
 * @details 使用QQuickWidget集成Qt Location的QML地图
 */

#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QQuickWidget>
#include <QGeoCoordinate>
#include <QMap>

class QQmlEngine;
class QQmlContext;

/**
 * @class MapWidget
 * @brief 地图显示组件
 * @details 集成Qt Location地图，支持：
 *          - 地图显示和交互（平移、缩放）
 *          - 添加地图标记（UAV目标）
 *          - 地理坐标与场景坐标转换
 */
class MapWidget : public QQuickWidget
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent = nullptr);
    ~MapWidget();

    /**
     * @brief 设置地图中心点
     * @param latitude 纬度
     * @param longitude 经度
     */
    void setCenter(double latitude, double longitude);

    /**
     * @brief 设置地图缩放级别
     * @param zoomLevel 缩放级别（通常10-18）
     */
    void setZoomLevel(double zoomLevel);

    /**
     * @brief 添加或更新UAV标记
     * @param uavId UAV ID
     * @param latitude 纬度
     * @param longitude 经度
     * @param name UAV名称
     */
    void addOrUpdateUavMarker(int uavId, double latitude, double longitude, const QString &name);

    /**
     * @brief 移除UAV标记
     * @param uavId UAV ID
     */
    void removeUavMarker(int uavId);

    /**
     * @brief 清除所有UAV标记
     */
    void clearAllMarkers();

    /**
     * @brief 设置UAV标记颜色（根据状态）
     * @param uavId UAV ID
     * @param color 颜色（如"red", "orange", "green"）
     */
    void setMarkerColor(int uavId, const QString &color);

signals:
    /**
     * @brief 地图上的UAV被点击
     * @param uavId UAV ID
     */
    void uavMarkerClicked(int uavId);

private:
    void initQml();

    QQmlEngine *m_engine;
    QQmlContext *m_context;
};

#endif // MAPWIDGET_H
