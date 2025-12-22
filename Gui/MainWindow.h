/**
 * @file MainWindow.h
 * @brief 主窗口类头文件
 * @details 定义应用程序的主窗口，集成仿真管理、PPI显示、UI控制等功能
 *          采用MVC架构，分离数据、视图和控制逻辑
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma once

#include "DraggableHeaderView.h"       // 新增：可拖动列表头
#include "MapWidget.h"                 // 新增：地图组件
#include "PPIDataManager.h"            // 新增：PPI数据管理器
#include "PPIGraphicsItem.h"           // 新增：PPI图元
#include "SortableTableWidgetItem.h"   // 新增：自定义排序表格项
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
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QResizeEvent>
#include <QTableWidget>
#include <QTimer>
#include <QWheelEvent>
#include <memory>   // 引入智能指针

/**
 * @enum DisplayMode
 * @brief 显示模式枚举
 * @details 定义应用程序的显示模式，支持不同的可视化方式
 *          设计原因：支持多种显示模式，提升用户体验和系统灵活性
 */
enum class DisplayMode
{
    RadarOnly,   // 纯雷达模式：只显示PPI雷达界面
    MapRadar     // 地图+雷达模式：同时显示地图和PPI雷达
};

/**
 * @class MainWindow
 * @brief 主窗口类
 * @details 应用程序的主窗口，负责集成所有功能模块
 *          设计原因：
 *          1. MVC架构：分离数据（SimulationManager）、视图（GraphicsView）、控制（MainWindow）
 *          2. 统一管理：集中管理所有UI组件和数据对象，便于协调
 *          3. 信号槽机制：使用Qt信号槽实现松耦合的组件通信
 *          4. 智能指针：使用智能指针管理资源，避免内存泄漏
 *          提升：
 *          - 可维护性：清晰的架构，便于理解和修改
 *          - 可扩展性：易于添加新功能和UI组件
 *          - 性能：使用智能指针和定时器优化，提升响应速度
 *          - 用户体验：支持多种显示模式和交互方式
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT   // Qt 宏，允许使用信号和槽 (Signals & Slots)

        public :
        /**
         * @brief 构造函数
         * @param parent 父窗口指针
         * @details 初始化主窗口，创建所有UI组件和数据对象
         *          提升：使用智能指针管理资源，自动处理内存
         */
        MainWindow(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     * @details 自动清理资源，智能指针会自动释放对象
     *          提升：无需手动delete，避免内存泄漏
     */
    ~MainWindow();

public slots:
    /**
     * @brief 定时器超时槽函数
     * @details 定期更新所有UAV位置和PPI显示
     *          设计原因：使用定时器实现周期性更新，保持显示同步
     *          提升：统一的时间控制，确保数据一致性
     */
    void updatePathTimeout();

    /**
     * @brief 切换仿真运行/暂停状态
     * @details 控制定时器的启动和停止，实现仿真的暂停和恢复
     *          提升：支持用户控制仿真进度，提升交互性
     */
    void toggleSimulation();

    /**
     * @brief 处理无人机点击事件
     * @param uavId 被点击的无人机ID
     * @details 当用户点击无人机时，高亮显示并更新相关信息
     *          提升：提升用户交互体验，便于选择和追踪目标
     */
    void onUavClicked(int uavId);

    /**
     * @brief 处理目标重点关注状态切换
     * @param targetId 目标ID
     * @param focused 是否为重点关注状态
     * @details 当目标的重点关注状态变化时更新UI显示
     *          提升：提供状态反馈，便于用户了解目标状态
     */
    void onTargetFocusToggled(int targetId, bool focused);

    /**
     * @brief 处理目标双击导引事件
     * @param targetId 被双击的目标ID
     * @details 当用户双击目标时，执行导引操作
     *          提升：提供快捷操作，提升用户效率
     */
    void onTargetDoubleClicked(int targetId);

    /**
     * @brief 处理目标导引状态切换
     * @param targetId 目标ID
     * @param guiding 是否为导引状态
     * @details 当目标的导引状态变化时更新UI显示
     *          提升：提供状态反馈，便于用户了解导引状态
     */
    void onTargetGuidanceToggled(int targetId, bool guiding);

    /**
     * @brief 切换显示模式
     * @details 在纯雷达模式和地图+雷达模式之间切换
     *          提升：支持多种显示方式，适应不同使用场景
     */
    void switchDisplayMode();

    /**
     * @brief 切换PPI拖动功能
     * @details 启用或禁用PPI图元的拖动功能
     *          提升：支持灵活的交互方式，提升用户体验
     */
    void togglePPIDrag();

    /**
     * @brief 处理表头排序变化
     * @param logicalIndex 被点击的列索引
     * @details 当用户点击表头排序时，记录当前排序列名
     *          支持列拖动后的正确排序
     */
    void onTableHeaderClicked(int logicalIndex);

    /**
     * @brief 处理PPI位置变化（拖动）
     * @details 当PPI被拖动时，同步移动地图而非PPI本身
     *          PPI圆心代表战车雷达位置，拖动查看周围区域
     */
    void onPPIPositionChanged();

protected:
    /**
     * @brief 窗口大小改变事件
     * @param event 事件对象
     * @details 当窗口大小改变时，同步调整地图和视图的大小
     */
    void resizeEvent(QResizeEvent *event) override;

    /**
     * @brief 鼠标滚轮事件
     * @param event 事件对象
     * @details 处理滚轮缩放，同步PPI距离环和地图缩放级别
     */
    void wheelEvent(QWheelEvent *event) override;

private:
    /**
     * @brief 初始化UI界面
     * @details 创建所有UI组件，设置布局和连接信号槽
     *          提升：集中管理UI初始化，便于维护
     */
    void setupUI();

    /**
     * @brief 更新遥测数据
     * @param uav 无人机模型指针
     * @details 更新侧边栏显示的遥测数据（位置、状态等）
     *          提升：实时显示无人机状态，便于监控
     */
    void updateTelemetry(UavModel *uav);

    /**
     * @brief 初始化无人机表格
     * @details 创建表格并填充初始数据
     *          提升：集中显示所有无人机信息，便于管理
     */
    void initUavTable();

    /**
     * @brief 更新表格中单个无人机的数据
     * @param uav 无人机模型指针
     * @details 更新指定无人机在表格中的显示数据
     *          提升：只更新变化的数据，提升性能
     */
    void updateUavRow(UavModel *uav);

    /**
     * @brief 更新表格中指定UAV的状态显示
     * @param uavId UAV的ID
     * @details 根据UAV的导引和重点关注状态更新表格显示
     *          使用Qt的UserRole设置排序键，实现智能排序
     */
    void updateUavStatus(int uavId);

    /**
     * @brief 更新标签下方显示的信息
     * @param uav 无人机模型指针
     * @details 更新无人机标签下方显示的详细信息
     *          提升：提供详细的目标信息，便于分析
     */
    void updateLabelInfo(UavModel *uav);

    /**
     * @brief 同步UAV数据到PPI
     * @details 将UAV模型数据转换为PPI显示格式并更新
     *          设计原因：保持UAV数据和PPI显示的同步
     *          提升：自动同步数据，确保显示一致性
     */
    void syncUavDataToPPI();

    /**
     * @brief 同步UAV数据到地图
     * @details 将UAV模型数据转换为地理坐标并更新到地图上
     *          设计原因：保持UAV数据和地图显示的同步
     *          提升：自动同步数据，确保地图显示一致性
     */
    void syncUavDataToMap();

    /**
     * @brief 应用纯雷达模式
     * @details 隐藏地图元素，只显示PPI雷达界面
     *          提升：聚焦雷达显示，减少视觉干扰
     */
    void applyRadarOnlyMode();

    /**
     * @brief 应用地图+雷达模式
     * @details 同时显示地图和PPI雷达，支持对比分析
     *          提升：提供多视角显示，便于综合分析
     */
    void applyMapRadarMode();

    /**
     * @brief 根据列名查找列索引
     * @param columnName 列名（表头文本）
     * @return 列索引，如果未找到返回-1
     * @details 支持列拖动后动态查找列索引，避免硬编码列位置
     *          提升：支持灵活的列顺序，提升用户体验
     */
    int getColumnIndexByName(const QString &columnName) const;

    /**
     * @brief 将场景坐标转换为地理坐标
     * @param sceneX 场景X坐标（笛卡尔坐标系）
     * @param sceneY 场景Y坐标（笛卡尔坐标系）
     * @param latitude 输出纬度
     * @param longitude 输出经度
     * @details 使用简单线性映射将场景坐标转换为经纬度
     *          提升：统一坐标转换，便于地图显示
     */
    void sceneToGeo(double sceneX, double sceneY, double &latitude, double &longitude) const;

    /**
     * @brief 根据PPI距离环计算地图缩放级别
     * @param huanJuMeters PPI距离环半径（米）
     * @return 地图缩放级别
     * @details 将PPI的距离环半径映射到地图的缩放级别
     */
    double calculateMapZoomLevel(double huanJuMeters) const;

    /**
     * @brief 同步地图中心到PPI中心（天安门）
     * @details 确保地图的中心与PPI的圆心对齐
     */
    void syncMapCenter();

    /**
     * @brief 根据UAV距离判断应该显示在PPI还是地图上
     * @param uavX UAV的X坐标
     * @param uavY UAV的Y坐标
     * @return true表示在PPI范围内，false表示在PPI范围外
     */
    bool isUavInPPIRange(double uavX, double uavY) const;

    // ========== 数据管理 ==========
    /**
     * @brief 仿真管理器
     * @details 管理所有无人机实例和仿真逻辑
     *          使用智能指针，自动管理内存
     */
    std::unique_ptr<SimulationManager> m_simManager;

    /**
     * @brief PPI数据管理器
     * @details 管理PPI显示所需的目标数据
     *          使用智能指针，自动管理内存
     */
    std::unique_ptr<PPIDataManager> m_ppiDataManager;

    /**
     * @brief 当前仿真步数
     * @details 记录当前仿真执行到第几个步骤
     *          用于控制UAV位置更新和航迹播放
     */
    int m_currentStep = 0;

    // ========== 定时器 ==========
    /**
     * @brief 更新路径定时器
     * @details 定期触发UAV位置更新和显示刷新
     *          使用智能指针，自动管理内存
     */
    std::unique_ptr<QTimer> updatePathTimer;

    // ========== 控制面板UI组件 ==========
    /**
     * @brief 状态标签
     * @details 显示当前仿真状态信息
     */
    QLabel *m_statusLabel = nullptr;

    /**
     * @brief 位置标签
     * @details 显示当前选中UAV的位置信息
     */
    QLabel *m_posLabel = nullptr;

    /**
     * @brief 目标详情标签
     * @details 显示选中目标的详细信息
     */
    QLabel *m_targetDetailLabel = nullptr;

    /**
     * @brief 控制按钮
     * @details 用于启动/暂停仿真
     */
    QPushButton *m_controlButton = nullptr;

    /**
     * @brief 距离环距离编辑框
     * @details 用于输入圆环环距离
     */
    QLineEdit *m_ringDistanceEdit = nullptr;
    /**
     * @brief 模式切换按钮
     * @details 用于切换显示模式（纯雷达/地图+雷达）
     */
    QPushButton *m_modeButton = nullptr;

    /**
     * @brief PPI拖动切换按钮
     * @details 用于启用/禁用PPI图元的拖动功能
     */
    QPushButton *m_dragButton = nullptr;

    /**
     * @brief 无人机状态表格
     * @details 以表格形式显示所有无人机的状态信息
     *          提升：集中显示，便于对比和管理
     */
    QTableWidget *m_uavTable = nullptr;

    // ========== QGraphicsView相关 ==========
    /**
     * @brief 图形场景
     * @details 管理所有图形图元的场景对象
     *          使用Qt对象树，自动管理内存
     */
    SimScene *m_scene = nullptr;

    /**
     * @brief 图形视图
     * @details 显示场景的视图对象，支持缩放、平移等操作
     *          使用Qt对象树，自动管理内存
     */
    SimView *m_view = nullptr;

    // ========== PPI图元 ==========
    /**
     * @brief PPI雷达显示图元
     * @details 负责绘制PPI雷达界面，包括目标、航迹、标尺等
     *          使用Qt对象树，自动管理内存
     */
    PPIGraphicsItem *m_ppiItem = nullptr;

    // ========== 地图组件 ==========
    /**
     * @brief 地图显示组件
     * @details 使用Qt Location显示地理地图和UAV位置
     *          使用Qt对象树，自动管理内存
     */
    MapWidget *m_mapWidget = nullptr;

    /**
     * @brief 层叠容器组件
     * @details 包含地图和视图的层叠容器，用于resize时同步大小
     */
    QWidget *m_stackWidget = nullptr;

    /**
     * @brief PPI上次位置
     * @details 用于计算拖动偏移量，从而移动地图
     */
    QPointF m_lastPPIPos;

    // ========== 传统图元（可选保留） ==========
    /**
     * @brief 路径图元指针（已废弃，保留以兼容旧代码）
     */
    PathItem *m_pathItem = nullptr;

    /**
     * @brief 无人机ID到目标点图元的映射
     * @details 用于快速查找和更新无人机图元
     *          提升：O(1)查找性能，提升更新效率
     */
    QMap<int, UavItem *> m_uavItemMap;

    /**
     * @brief 无人机ID到标签图元的映射
     * @details 用于快速查找和更新无人机标签
     *          提升：O(1)查找性能，提升更新效率
     */
    QMap<int, UavLabelItem *> m_uavLabelMap;

    /**
     * @brief 无人机ID到表格行号的映射
     * @details 用于快速定位无人机在表格中的位置
     *          提升：O(1)查找性能，提升更新效率
     */
    QMap<int, int> m_uavRowMap;

    /**
     * @brief 无人机ID到路径图元的映射
     * @details 用于快速查找和更新无人机航迹图元
     *          提升：O(1)查找性能，提升更新效率
     */
    QMap<int, PathItem *> m_pathItemMap;

    // ========== 显示模式 ==========
    /**
     * @brief 当前显示模式
     * @details 记录当前使用的显示模式，默认地图+雷达模式
     *          提升：支持模式切换，提升系统灵活性
     */
    DisplayMode m_displayMode = DisplayMode::MapRadar;

    // ========== 表格排序 ==========
    /**
     * @brief 当前排序的列名
     * @details 记录当前用于排序的列名（而非索引），以支持列拖动后的正确排序
     *          当列被拖动时，通过列名动态查找新的列索引进行排序
     */
    QString m_sortColumnName = "Status";

    // ========== 坐标转换参数 ==========
    /**
     * @brief 地图参考点纬度（原点对应的纬度）
     * @details 场景坐标(0,0)对应的纬度，默认为天安门
     */
    const double m_refLatitude = 39.9042;

    /**
     * @brief 地图参考点经度（原点对应的经度）
     * @details 场景坐标(0,0)对应的经度，默认为天安门
     */
    const double m_refLongitude = 116.4074;

    /**
     * @brief 坐标转换比例尺（米/度）
     * @details 在北纬40度附近，1度纬度约111km，1度经度约85km
     *          这里使用平均值约100km/度，即1000米约0.01度
     */
    const double m_metersPerDegreeLat = 111000.0;   // 纬度方向：约111km/度
    const double m_metersPerDegreeLon = 85000.0;    // 经度方向：约85km/度（北纬40度附近）
};
#endif   // UAVITEM_H
