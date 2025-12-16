#include "MainWindow.h"
#include "trajectorygenerator.h"
#include <QDebug>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>
#include <limits>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    updatePathTimer = std::make_unique<QTimer>(this);
    connect(updatePathTimer.get(), &QTimer::timeout, this, &MainWindow::updatePathTimeout, Qt::QueuedConnection);

    // 1. 初始化 Service 模块的业务对象
    m_simManager = std::make_unique<SimulationManager>();

    // 2. 初始化 PPI 数据管理器
    m_ppiDataManager = std::make_unique<PPIDataManager>(this);

    setupUI();

    // 连接场景中点击无人机的信号
    connect(m_scene, &SimScene::uavClicked, this, &MainWindow::onUavClicked);

    // 连接PPI图元的信号
    if (m_ppiItem) {
        connect(m_ppiItem, &PPIGraphicsItem::targetClicked, this, &MainWindow::onUavClicked);
        connect(m_ppiItem, &PPIGraphicsItem::targetDoubleClicked, this, [this](int id) { qDebug() << "Target" << id << "double clicked - 可以添加导引等功能"; });
    }

    updatePathTimer->start(1000);
}

MainWindow::~MainWindow()
{
    // 这里的 m_uav 会被 std::unique_ptr 自动释放，不需要手动 delete
    // QGraphicsView/Scene都会被其parent自动释放，无需手动delete
    // 只有非QObject的指针或裸指针才需要手动delete
    //    delete m_simManager;
    //    delete updatePathTimer;
}

void MainWindow::updatePathTimeout()
{
    // 先更新所有UAV的位置
    for (const auto &uav : m_simManager->getUavs()) {
        uav->updatePosition(m_currentStep);
        uav->setCurrentPoint(m_currentStep % uav->getPath().size());
    }

    // 同步数据到PPI
    syncUavDataToPPI();

    // 更新传统图元和UI（仅更新可见区域）
    QRectF visibleRect = m_view->mapToScene(m_view->viewport()->rect()).boundingRect();
    for (const auto &uav : m_simManager->getUavs()) {
        QPointF pos(uav->getX(), uav->getY());

        // 只更新可见区域的UAV
        if (visibleRect.contains(pos)) {
            // 更新图元位置
            auto it = m_uavItemMap.find(uav->getId());
            if (it != m_uavItemMap.end()) {
                it.value()->setPos(uav->getX(), uav->getY());
            }

            // 更新标签虚线指向的无人机位置
            auto labelIt = m_uavLabelMap.find(uav->getId());
            if (labelIt != m_uavLabelMap.end()) {
                labelIt.value()->setUavScenePos(QPointF(uav->getX(), uav->getY()));
            }

            // 更新侧边栏/表格中的遥测数据
            updateTelemetry(uav.get());
            updateUavRow(uav.get());
            updateLabelInfo(uav.get());
        }
    }

    // 步进计数器（在循环外递增）
    m_currentStep++;
    if (m_currentStep >= std::numeric_limits<int>::max()) {
        m_currentStep = 0;
    }
}

void MainWindow::toggleSimulation()
{
    if (updatePathTimer->isActive()) {
        // 计时器正在运行，则暂停
        updatePathTimer->stop();
        m_controlButton->setText("▶ Resume Simulation");
    }
    else {
        // 计时器已停止，则启动
        updatePathTimer->start(1000);
        m_controlButton->setText("❚❚ Pause Simulation");
    }
}

void MainWindow::setupUI()
{
    // 1. Graphics View setup
    m_scene = new SimScene(this);
    m_view  = new SimView(m_scene, this);

    // 设置场景的边界（坐标范围）
    m_scene->setSceneRect(-20000, -20000, 40000, 40000);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_view->setRenderHint(QPainter::Antialiasing);

    // ========== 新增：创建PPI图元 ==========
    m_ppiItem = new PPIGraphicsItem();
    m_ppiItem->setRadius(340);    // 设置PPI半径
    m_ppiItem->setHuanJu(1000);   // 设置距离环为1km，使当前仿真尺度下目标分布更均匀
    m_ppiItem->setPos(0, 0);      // 设置PPI位置在场景中心
    m_ppiItem->setZValue(-10);    // 设置为最底层
    m_scene->addItem(m_ppiItem);
    // =======================================

    // --- 2. 遥测控制台 (QDockWidget) Setup ---
    QDockWidget *controlDock = new QDockWidget("Simulation Control", this);
    controlDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, controlDock);
    controlDock->setWindowFlags(Qt::FramelessWindowHint);

    // 去掉标题栏
    QWidget *titleBarWidget = controlDock->titleBarWidget();
    QWidget *lEmptyWidget   = new QWidget();
    controlDock->setTitleBarWidget(lEmptyWidget);
    delete titleBarWidget;

    QWidget     *dockContents = new QWidget(controlDock);
    QGridLayout *layout       = new QGridLayout(dockContents);

    // --- 2a. 控制按钮 ---
    m_controlButton = new QPushButton("▶ Start Simulation");
    layout->addWidget(new QLabel("Simulation State:"), 0, 0);
    layout->addWidget(m_controlButton, 0, 1);

    // --- 2b. 模式切换按钮 ---
    m_modeButton = new QPushButton("Switch to Radar-Only Mode");
    layout->addWidget(new QLabel("Display Mode:"), 1, 0);
    layout->addWidget(m_modeButton, 1, 1);

    // --- 2c. PPI拖动切换按钮 ---
    m_dragButton = new QPushButton("Enable PPI Drag");
    layout->addWidget(new QLabel("PPI Drag:"), 2, 0);
    layout->addWidget(m_dragButton, 2, 1);

    // --- 2d. 多架无人机遥测数据显示表格 ---
    m_uavTable = new QTableWidget(dockContents);
    initUavTable();
    layout->addWidget(new QLabel("UAV Telemetry:"), 3, 0, 1, 2);
    layout->addWidget(m_uavTable, 4, 0, 1, 2);

    dockContents->setLayout(layout);
    controlDock->setWidget(dockContents);

    // 3. 布局与交互
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *vlayout   = new QVBoxLayout(centralWidget);
    QLabel      *infoLabel = new QLabel("PPI Radar Display - Ready to fly...", this);

    vlayout->addWidget(m_view);
    vlayout->addWidget(infoLabel);
    connect(m_controlButton, &QPushButton::clicked, this, &MainWindow::toggleSimulation);
    connect(m_modeButton, &QPushButton::clicked, this, &MainWindow::switchDisplayMode);
    connect(m_dragButton, &QPushButton::clicked, this, &MainWindow::togglePPIDrag);

    // 4. 创建传统图元（可选，用于对比）
    //    for (const auto &uav : m_simManager->getUavs()) {
    //        // 创建PathItem并添加到场景
    //        auto *pathItem              = new PathItem(uav->getPath());
    //        m_pathItemMap[uav->getId()] = pathItem;
    //        m_scene->addItem(pathItem);

    //        // 创建UavItem 并添加到场景（目标点）
    //        auto *uavItem = new UavItem();
    //        uavItem->setId(uav->getId());
    //        m_uavItemMap[uav->getId()] = uavItem;
    //        m_scene->addItem(uavItem);
    //        uavItem->setPos(uav->getX(), uav->getY());

    //        // 创建标签图元，并与无人机通过虚线连接
    //        auto *labelItem = new UavLabelItem();
    //        labelItem->setId(uav->getId());
    //        labelItem->setName(uav->getName());
    //        labelItem->setUavScenePos(QPointF(uav->getX(), uav->getY()));
    //        m_uavLabelMap[uav->getId()] = labelItem;
    //        m_scene->addItem(labelItem);

    //        // 初始化遥测、表格和标签信息
    //        updateTelemetry(uav.get());
    //        updateUavRow(uav.get());
    //        updateLabelInfo(uav.get());
    //    }

    resize(1400, 800);   // 调整窗口大小
}

void MainWindow::updateTelemetry(UavModel *uav)
{
    if (!m_statusLabel || !m_posLabel)
        return;

    m_statusLabel->setText(QString::number(m_currentStep));
    QString posText = QString("(%1, %2)").arg(QString::number(uav->getX(), 'f', 2)).arg(QString::number(uav->getY(), 'f', 2));
    m_posLabel->setText(posText);
}

void MainWindow::initUavTable()
{
    if (!m_simManager)
        return;

    const auto &uavs = m_simManager->getUavs();
    m_uavTable->setColumnCount(4);
    m_uavTable->setRowCount(static_cast<int>(uavs.size()));
    QStringList headers;
    headers << "ID"
            << "Name"
            << "X"
            << "Y";
    m_uavTable->setHorizontalHeaderLabels(headers);
    m_uavTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_uavTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_uavTable->setSelectionMode(QAbstractItemView::SingleSelection);

    int row = 0;
    for (const auto &uav : uavs) {
        int id          = uav->getId();
        m_uavRowMap[id] = row;

        m_uavTable->setItem(row, 0, new QTableWidgetItem(QString::number(id)));
        m_uavTable->setItem(row, 1, new QTableWidgetItem(uav->getName()));
        m_uavTable->setItem(row, 2, new QTableWidgetItem(QString::number(uav->getX(), 'f', 2)));
        m_uavTable->setItem(row, 3, new QTableWidgetItem(QString::number(uav->getY(), 'f', 2)));
        ++row;
    }
}

void MainWindow::updateUavRow(UavModel *uav)
{
    if (!m_uavTable)
        return;

    int id = uav->getId();
    if (!m_uavRowMap.contains(id))
        return;

    int row = m_uavRowMap.value(id);

    // 只更新位置相关列
    if (auto *itemX = m_uavTable->item(row, 2)) {
        itemX->setText(QString::number(uav->getX(), 'f', 2));
    }
    if (auto *itemY = m_uavTable->item(row, 3)) {
        itemY->setText(QString::number(uav->getY(), 'f', 2));
    }
}

void MainWindow::onUavClicked(int uavId)
{
    if (!m_simManager || !m_uavTable)
        return;

    // 高亮表格中对应行
    if (m_uavRowMap.contains(uavId)) {
        int row = m_uavRowMap.value(uavId);
        m_uavTable->selectRow(row);
        m_uavTable->scrollToItem(m_uavTable->item(row, 0));
    }

    // 查找对应的 UavModel，更新标签下方的信息并高亮该标签
    const auto &uavs = m_simManager->getUavs();
    for (const auto &uavPtr : uavs) {
        if (uavPtr->getId() == uavId) {
            // 先关闭其他标签的信息显示
            for (auto label : m_uavLabelMap) {
                label->setShowInfo(false);
            }
            // 打开当前标签的信息显示
            if (auto label = m_uavLabelMap.value(uavId, nullptr)) {
                label->setShowInfo(true);
                updateLabelInfo(uavPtr.get());
            }
            break;
        }
    }
}

void MainWindow::updateLabelInfo(UavModel *uav)
{
    if (!uav)
        return;
    auto *label = m_uavLabelMap.value(uav->getId(), nullptr);
    if (!label || !label->showInfo())
        return;

    QString info = QString("(%1, %2)").arg(QString::number(uav->getX(), 'f', 2)).arg(QString::number(uav->getY(), 'f', 2));
    label->setInfoText(info);
}

void MainWindow::syncUavDataToPPI()
{
    if (!m_ppiItem || !m_ppiDataManager)
        return;
    QPointF ppiCenter = m_ppiItem->pos();   // PPI在场景中的位置
    // 获取PPI参数（这里只关心PPI自身的显示半径和量程）
    // 注意：PPI在场景中的平移由QGraphicsItem自身的transform处理，
    //       数据层统一以(0,0)作为雷达中心，避免目标跟着PPI拖动而重新投影。
    double   radius = m_ppiItem->getRadius();
    uint32_t huanJu = m_ppiItem->getHuanJu();

    // 更新UAV数据到PPI数据管理器
    // 这里固定以(0,0)作为雷达中心来计算极坐标，PPI图元的位置只通过QGraphicsItem平移来体现。
    // 这样可以保证：拖动PPI时，PPI上的目标和航迹整体一起移动，而不是重新计算方位/距离导致“飘动”。
    m_ppiDataManager->updateFromUavs(m_simManager->getUavs(),
                                     0.0,   // 雷达中心X（数据坐标系）
                                     0.0,   // 雷达中心Y（数据坐标系）
                                     radius,
                                     huanJu);

    // 获取更新后的数据并同步到PPI图元
    LockedHash<Mubiao> &sourceData = m_ppiDataManager->getMubiaoHash();
    LockedHash<Mubiao> &targetData = m_ppiItem->getMubiaoHash();

    // 清空旧数据
    targetData.clear();

    // 批量复制新数据
    QList<int> keys = sourceData.keys();
    for (int id : keys) {
        QSharedPointer<Mubiao> mubiao = sourceData.value(id);
        if (mubiao) {
            targetData.insert(id, mubiao);
        }
    }

    // 触发PPI图元重绘
    m_ppiItem->update();
}

void MainWindow::switchDisplayMode()
{
    if (m_displayMode == DisplayMode::MapRadar) {
        // 切换到纯雷达模式
        m_displayMode = DisplayMode::RadarOnly;
        applyRadarOnlyMode();
        m_modeButton->setText("Switch to Map+Radar Mode");
    }
    else {
        // 切换到地图+雷达模式
        m_displayMode = DisplayMode::MapRadar;
        applyMapRadarMode();
        m_modeButton->setText("Switch to Radar-Only Mode");
    }
}

void MainWindow::applyRadarOnlyMode()
{
    if (!m_ppiItem || !m_scene || !m_view)
        return;

    // 1. 隐藏网格背景
    m_scene->setShowGrid(false);

    // 2. 放大PPI半径
    m_ppiItem->setRadius(900);
    m_ppiItem->setPPIOpacity(1.0);
    m_ppiItem->setDrawBackground(true);
    m_ppiItem->setZValue(10);   // 提升到前景

    // 3. 隐藏传统图元
    for (auto *pathItem : m_pathItemMap) {
        if (pathItem)
            pathItem->setVisible(false);
    }
    for (auto *uavItem : m_uavItemMap) {
        if (uavItem)
            uavItem->setVisible(false);
    }
    for (auto *labelItem : m_uavLabelMap) {
        if (labelItem)
            labelItem->setVisible(false);
    }

    // 4. 调整视图（可选）
    m_view->resetTransform();
    m_view->scale(0.8, 0.8);
    m_view->centerOn(0, 0);

    qDebug() << "Switched to Radar-Only Mode";
}

void MainWindow::applyMapRadarMode()
{
    if (!m_ppiItem || !m_scene || !m_view)
        return;

    // 1. 显示网格背景
    m_scene->setShowGrid(true);

    // 2. 缩小PPI半径，添加透明度
    m_ppiItem->setRadius(400);
    m_ppiItem->setPPIOpacity(0.6);
    m_ppiItem->setDrawBackground(false);
    m_ppiItem->setZValue(-10);   // 降到背景层

    // 3. 显示传统图元
    for (auto *pathItem : m_pathItemMap) {
        if (pathItem)
            pathItem->setVisible(true);
    }
    for (auto *uavItem : m_uavItemMap) {
        if (uavItem)
            uavItem->setVisible(true);
    }
    for (auto *labelItem : m_uavLabelMap) {
        if (labelItem)
            labelItem->setVisible(true);
    }

    // 4. 恢复视图
    m_view->resetTransform();
    m_view->scale(1.0, 1.0);
    m_view->centerOn(0, 0);

    qDebug() << "Switched to Map+Radar Mode";
}

void MainWindow::togglePPIDrag()
{
    if (!m_ppiItem)
        return;

    bool currentState = m_ppiItem->getDraggable();
    bool newState     = !currentState;

    m_ppiItem->setDraggable(newState);

    // 更新按钮文本
    if (newState) {
        m_dragButton->setText("Disable PPI Drag");
        qDebug() << "PPI Drag Enabled - You can now drag the radar display";
    }
    else {
        m_dragButton->setText("Enable PPI Drag");
        qDebug() << "PPI Drag Disabled - Click targets to select them";
    }
}
