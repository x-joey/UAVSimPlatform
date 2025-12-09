#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma once

#include "UavModel.h"
#include "pathitem.h"
#include "simscene.h"
#include "simulationmanager.h"
#include "simview.h"
#include "uavitem.h"
#include "uavlabelitem.h"
#include "PPIGraphicsItem.h"        // 新增：PPI图元
#include "PPIDataManager.h"         // 新增：PPI数据管理器

#include <QDockWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <memory>   // 引入智能指针

// 显示模式枚举
enum class DisplayMode {
    RadarOnly,      // 纯雷达模式
    MapRadar        // 地图+雷达模式
};

class MainWindow : public QMainWindow
{
    Q_OBJECT   // Qt 宏，允许使用信号和槽 (Signals & Slots)

        public : MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void updatePathTimeout();
    void toggleSimulation();
    void onUavClicked(int uavId);
    void switchDisplayMode();  // 新增：切换显示模式
    void togglePPIDrag();      // 新增：切换PPI拖动

private:
    void setupUI();                        // 初始化UI
    void updateTelemetry(UavModel *uav);   // 负责更新遥测数据（侧边文本）
    void initUavTable();                   // 初始化无人机表格
    void updateUavRow(UavModel *uav);      // 更新单个无人机在表格中的数据
    void updateLabelInfo(UavModel *uav);   // 更新标签下方展示的信息
    void syncUavDataToPPI();               // 新增：同步UAV数据到PPI

    // 显示模式控制
    void applyRadarOnlyMode();             // 应用纯雷达模式
    void applyMapRadarMode();              // 应用地图+雷达模式

    // 数据管理
    std::unique_ptr<SimulationManager> m_simManager;
    std::unique_ptr<PPIDataManager> m_ppiDataManager;  // 新增：PPI数据管理器
    int m_currentStep = 0;                              // 记录当前飞到第几个点了

    // 定时器
    std::unique_ptr<QTimer> updatePathTimer;

    // 控制面板
    QLabel       *m_statusLabel   = nullptr;
    QLabel       *m_posLabel      = nullptr;
    QPushButton  *m_controlButton = nullptr;
    QPushButton  *m_modeButton    = nullptr;           // 新增：模式切换按钮
    QPushButton  *m_dragButton    = nullptr;           // 新增：PPI拖动切换按钮
    QTableWidget *m_uavTable      = nullptr;           // 展示多架无人机状态的表格

    // QGraphicsView相关
    SimScene *m_scene    = nullptr;                    // 场景
    SimView  *m_view     = nullptr;                    // 视图

    // PPI图元（新增）
    PPIGraphicsItem *m_ppiItem = nullptr;              // PPI雷达显示图元

    // 传统图元（可选保留）
    PathItem *m_pathItem = nullptr;                    // 路径图元指针
    QMap<int, UavItem *>      m_uavItemMap;            // 无人机 ID -> 目标点图元
    QMap<int, UavLabelItem *> m_uavLabelMap;           // 无人机 ID -> 标签图元
    QMap<int, int>            m_uavRowMap;             // 无人机 ID -> 表格行号
    QMap<int, PathItem *>     m_pathItemMap;           // 无人机 ID -> 路径图元

    // 显示模式
    DisplayMode m_displayMode = DisplayMode::MapRadar; // 默认地图+雷达模式
};
#endif   // UAVITEM_H
