/**
 * @file MapWidget.cpp
 * @brief Qt Location地图组件实现
 */

#include "MapWidget.h"
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QDebug>

MapWidget::MapWidget(QWidget *parent)
    : QQuickWidget(parent)
    , m_engine(nullptr)
    , m_context(nullptr)
{
    // 设置Qt Quick渲染设置
    setResizeMode(QQuickWidget::SizeRootObjectToView);

    // 初始化QML
    initQml();
}

MapWidget::~MapWidget()
{
}

void MapWidget::initQml()
{
    m_engine = engine();
    m_context = rootContext();

    // 设置QML可访问的属性
    m_context->setContextProperty("mapWidget", this);

    // 加载QML文件
    setSource(QUrl(QStringLiteral("qrc:/qml/MapView.qml")));

    if (status() == QQuickWidget::Error) {
        qWarning() << "Failed to load MapView.qml:" << errors();
    }
}

void MapWidget::setCenter(double latitude, double longitude)
{
    QQuickItem *rootItem = rootObject();
    if (rootItem) {
        QMetaObject::invokeMethod(rootItem, "setMapCenter",
                                  Q_ARG(QVariant, latitude),
                                  Q_ARG(QVariant, longitude));
    }
}

void MapWidget::setZoomLevel(double zoomLevel)
{
    QQuickItem *rootItem = rootObject();
    if (rootItem) {
        QMetaObject::invokeMethod(rootItem, "setZoomLevel",
                                  Q_ARG(QVariant, zoomLevel));
    }
}

void MapWidget::addOrUpdateUavMarker(int uavId, double latitude, double longitude, const QString &name)
{
    QQuickItem *rootItem = rootObject();
    if (rootItem) {
        QMetaObject::invokeMethod(rootItem, "addOrUpdateMarker",
                                  Q_ARG(QVariant, uavId),
                                  Q_ARG(QVariant, latitude),
                                  Q_ARG(QVariant, longitude),
                                  Q_ARG(QVariant, name));
    }
}

void MapWidget::removeUavMarker(int uavId)
{
    QQuickItem *rootItem = rootObject();
    if (rootItem) {
        QMetaObject::invokeMethod(rootItem, "removeMarker",
                                  Q_ARG(QVariant, uavId));
    }
}

void MapWidget::clearAllMarkers()
{
    QQuickItem *rootItem = rootObject();
    if (rootItem) {
        QMetaObject::invokeMethod(rootItem, "clearMarkers");
    }
}

void MapWidget::setMarkerColor(int uavId, const QString &color)
{
    QQuickItem *rootItem = rootObject();
    if (rootItem) {
        QMetaObject::invokeMethod(rootItem, "setMarkerColor",
                                  Q_ARG(QVariant, uavId),
                                  Q_ARG(QVariant, color));
    }
}
