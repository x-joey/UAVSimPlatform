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

#include <QDockWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <memory>   // 引入智能指针
    class MainWindow : public QMainWindow
{
    Q_OBJECT   // Qt 宏，允许使用信号和槽 (Signals & Slots)

        public : MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void updatePathTimeout();
    void toggleSimulation();
    void onUavClicked(int uavId);

private:
    void setupUI();                        // 初始化UI
    void updateTelemetry(UavModel *uav);   // 负责更新遥测数据（侧边文本）
    void initUavTable();                   // 初始化无人机表格
    void updateUavRow(UavModel *uav);      // 更新单个无人机在表格中的数据
    void updateLabelInfo(UavModel *uav);   // 更新标签下方展示的信息

    // 使用智能指针管理 Core 模块的对象
    // std::unique_ptr 类似于 Java 的对象引用，但它会在 MainWindow 销毁时自动 delete uav，防止内存泄漏
    //    std::unique_ptr<UavModel> m_uav;
    SimulationManager *m_simManager    = nullptr;
    int                m_currentStep   = 0;   // 记录当前飞到第几个点了
    QTimer            *updatePathTimer = nullptr;
    QLabel            *m_statusLabel   = nullptr;
    QLabel            *m_posLabel      = nullptr;
    QPushButton       *m_controlButton = nullptr;
    QTableWidget      *m_uavTable      = nullptr;   // 展示多架无人机状态的表格
    //    QPushButton              *btnMove         = nullptr;

    // QGrahpicsView
    SimScene *m_scene    = nullptr;            // 场景
    SimView  *m_view     = nullptr;            // 视图
    PathItem *m_pathItem = nullptr;            // 路径图元指针
                                               //    UavItem  *m_uavItem  = nullptr;   // 无人机图元指针
    QMap<int, UavItem *>      m_uavItemMap;    // 无人机 ID -> 目标点图元
    QMap<int, UavLabelItem *> m_uavLabelMap;   // 无人机 ID -> 标签图元
    QMap<int, int>            m_uavRowMap;     // 无人机 ID -> 表格行号
};
#endif   // UAVITEM_H
