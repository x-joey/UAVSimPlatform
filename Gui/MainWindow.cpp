#include "MainWindow.h"
#include "trajectorygenerator.h"
#include <QDebug>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QQuickWindow>
#include <QSizePolicy>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <cmath>
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
        connect(m_ppiItem, &PPIGraphicsItem::targetClicked, this, &MainWindow::onUavClicked, Qt::QueuedConnection);
        connect(m_ppiItem, &PPIGraphicsItem::targetDoubleClicked, this, &MainWindow::onTargetDoubleClicked, Qt::QueuedConnection);
        connect(m_ppiItem, &PPIGraphicsItem::targetFocusToggled, this, &MainWindow::onTargetFocusToggled, Qt::QueuedConnection);
        connect(m_ppiItem, &PPIGraphicsItem::targetGuidanceToggled, this, &MainWindow::onTargetGuidanceToggled, Qt::QueuedConnection);
    }

    // 连接地图组件的信号
    if (m_mapWidget) {
        connect(m_mapWidget, &MapWidget::uavMarkerClicked, this, &MainWindow::onUavClicked, Qt::QueuedConnection);
    }

    // 初始化PPI位置
    if (m_ppiItem) {
        m_lastPPIPos = m_ppiItem->pos();
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
    // 检查PPI位置变化（拖动检测）
    if (m_ppiItem && m_ppiItem->getDraggable()) {
        QPointF currentPos = m_ppiItem->pos();
        if (currentPos != m_lastPPIPos) {
            onPPIPositionChanged();
            m_lastPPIPos = currentPos;
        }
    }

    // 先更新所有UAV的位置
    for (const auto &uav : m_simManager->getUavs()) {
        uav->updatePosition(m_currentStep);
        uav->setCurrentPoint(m_currentStep % uav->getPath().size());
    }

    // 同步数据到PPI
    syncUavDataToPPI();

    // 同步数据到地图
    if (m_displayMode == DisplayMode::MapRadar && m_mapWidget) {
        syncUavDataToMap();
    }

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
    // 禁用默认的拖拽模式，后续通过自定义方式控制地图移动
    m_view->setDragMode(QGraphicsView::NoDrag);
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_view->setRenderHint(QPainter::Antialiasing);

    // ========== 新增：创建PPI图元 ==========
    m_ppiItem = new PPIGraphicsItem();
    m_ppiItem->setRadius(400);             // 设置PPI半径（稍大以便在地图上可见）
    m_ppiItem->setHuanJu(1000);            // 设置距离环为1km，使当前仿真尺度下目标分布更均匀
    m_ppiItem->setPos(0, 0);               // 设置PPI位置在场景中心
    m_ppiItem->setZValue(10);              // 设置为上层，覆盖在地图上
    m_ppiItem->setPPIOpacity(0.7);         // 设置半透明，让地图可透过PPI显示
    m_ppiItem->setDrawBackground(false);   // 不绘制PPI背景，使用地图作为背景
    m_scene->addItem(m_ppiItem);
    // =======================================

    // ========== 新增：创建地图组件 ==========
    m_mapWidget = new MapWidget(this);
    m_mapWidget->setMinimumSize(400, 400);
    m_mapWidget->setCenter(m_refLatitude, m_refLongitude);
    m_mapWidget->setZoomLevel(14);
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

    // --- 2d. 目标详情标签 ---
    m_targetDetailLabel = new QLabel("Click on a target to see details", dockContents);
    m_targetDetailLabel->setWordWrap(true);
    m_targetDetailLabel->setMinimumHeight(150);                        // 设置最小高度
    m_targetDetailLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);   // 左上对齐
    m_targetDetailLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 8px; border: 1px solid #ccc; border-radius: 3px; }");
    layout->addWidget(new QLabel("Target Details:"), 3, 0, 1, 2);
    layout->addWidget(m_targetDetailLabel, 4, 0, 1, 2);

    // --- 2e. 多架无人机遥测数据显示表格 ---
    m_uavTable = new QTableWidget(dockContents);
    m_uavTable->setStyleSheet("QTableWidget::item:selected { "
                              "background-color: #66ccff; "
                              "color: white; "
                              "} "

                              "QTableWidget::item:focus { "
                              "background-color: #ffcc00; "
                              "border: 1px solid #ff6600; "
                              "color: black; "
                              "} "

                              // 鼠标悬停时的高亮效果
                              "QTableWidget::item:hover { "
                              "background-color: #cce6ff; "   // 悬停时的浅蓝色
                              "}");

    // 创建可拖动列的表头
    DraggableHeaderView *draggableHeader = new DraggableHeaderView(Qt::Horizontal, m_uavTable);
    draggableHeader->setColumnDraggingEnabled(true);
    m_uavTable->setHorizontalHeader(draggableHeader);

    // 连接列交换信号（可选，用于调试或保存列顺序）
    connect(draggableHeader, &DraggableHeaderView::columnSwapped, this, [](int oldIndex, int newIndex) { qDebug() << "User swapped columns:" << oldIndex << "<->" << newIndex; });

    // 连接表头点击信号，记录排序列
    connect(m_uavTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::onTableHeaderClicked);

    initUavTable();
    layout->addWidget(new QLabel("UAV Telemetry:"), 5, 0, 1, 2);
    layout->addWidget(m_uavTable, 6, 0, 1, 2);

    //
    m_ringDistanceEdit = new QLineEdit(this);
    layout->addWidget(new QLabel("Ring Distance(m):"), 7, 0, 1, 1);
    layout->addWidget(m_ringDistanceEdit, 7, 1, 1, 1);

    dockContents->setLayout(layout);
    controlDock->setWidget(dockContents);

    // 3. 布局与交互
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建层叠容器：地图在底层，PPI视图在上层
    m_stackWidget = new QWidget(centralWidget);
    m_stackWidget->setMinimumSize(800, 600);
    m_stackWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 使用绝对定位实现层叠
    m_mapWidget->setParent(m_stackWidget);
    m_view->setParent(m_stackWidget);

    // 设置视图背景透明，让地图可以透过来显示
    m_view->setStyleSheet("background: transparent;");
    m_view->viewport()->setStyleSheet("background: transparent;");
    m_view->setFrameStyle(QFrame::NoFrame);

    // 设置场景背景透明
    m_scene->setBackgroundBrush(Qt::transparent);

    // 地图和视图初始大小会在 resizeEvent 中设置
    m_mapWidget->lower();   // 地图在底层
    m_view->raise();        // PPI视图在上层

    QVBoxLayout *vlayout   = new QVBoxLayout(centralWidget);
    QLabel      *infoLabel = new QLabel("PPI Radar Display overlayed on Map - Ready to fly...", this);

    vlayout->addWidget(m_stackWidget);
    vlayout->addWidget(infoLabel);
    connect(m_controlButton, &QPushButton::clicked, this, &MainWindow::toggleSimulation);
    connect(m_modeButton, &QPushButton::clicked, this, &MainWindow::switchDisplayMode);
    connect(m_dragButton, &QPushButton::clicked, this, &MainWindow::togglePPIDrag);
    connect(m_ringDistanceEdit, &QLineEdit::returnPressed, this, [=]() {
        int huanju = m_ringDistanceEdit->text().toInt();
        m_ppiItem->setHuanJu(huanju);
        qDebug() << "环距已输入" << endl;
    });
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

    // 手动触发一次布局更新，确保地图和视图大小正确
    QTimer::singleShot(100, this, [this]() {
        if (m_stackWidget && m_mapWidget && m_view) {
            QSize stackSize = m_stackWidget->size();
            m_mapWidget->setGeometry(0, 0, stackSize.width(), stackSize.height());
            m_view->setGeometry(0, 0, stackSize.width(), stackSize.height());
            qDebug() << "Initial layout: Stack size =" << stackSize;
        }

        // 同步地图中心到PPI圆心
        syncMapCenter();
    });
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
    m_uavTable->setColumnCount(5);   // 增加一列用于显示状态
    m_uavTable->setRowCount(static_cast<int>(uavs.size()));
    QStringList headers;
    headers << "ID"
            << "Name"
            << "X"
            << "Y"
            << "Status";   // 新增状态列
    m_uavTable->setHorizontalHeaderLabels(headers);
    m_uavTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_uavTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_uavTable->setSelectionMode(QAbstractItemView::SingleSelection);

    // 启用排序功能
    m_uavTable->setSortingEnabled(false);   // 先禁用排序，填充完数据后再启用

    int row = 0;
    for (const auto &uav : uavs) {
        int id          = uav->getId();
        m_uavRowMap[id] = row;

        m_uavTable->setItem(row, 0, new QTableWidgetItem(QString::number(id)));
        m_uavTable->setItem(row, 1, new QTableWidgetItem(uav->getName()));
        m_uavTable->setItem(row, 2, new QTableWidgetItem(QString::number(uav->getX(), 'f', 2)));
        m_uavTable->setItem(row, 3, new QTableWidgetItem(QString::number(uav->getY(), 'f', 2)));

        // 初始化状态列（使用自定义排序表格项）
        SortableTableWidgetItem *statusItem = new SortableTableWidgetItem("Normal");
        statusItem->setData(Qt::UserRole, 2);   // 设置排序键：Normal=2
        m_uavTable->setItem(row, 4, statusItem);
        ++row;
    }

    // 填充完数据后启用排序并按状态列排序
    m_uavTable->setSortingEnabled(true);
    m_sortColumnName = "Status";   // 记录排序列名
    int statusCol    = getColumnIndexByName(m_sortColumnName);
    if (statusCol >= 0) {
        m_uavTable->sortItems(statusCol, Qt::AscendingOrder);   // 按状态列升序排序（0=Guidance, 1=Priority, 2=Normal）
    }
}

void MainWindow::updateUavRow(UavModel *uav)
{
    if (!m_uavTable)
        return;

    int id = uav->getId();
    if (!m_uavRowMap.contains(id))
        return;

    // 动态获取列索引
    int idCol = getColumnIndexByName("ID");
    int xCol  = getColumnIndexByName("X");
    int yCol  = getColumnIndexByName("Y");

    if (idCol < 0 || xCol < 0 || yCol < 0)
        return;   // 列名不存在

    // 注意：由于启用了排序，需要通过ID查找实际行号
    // 遍历所有行查找匹配的ID
    for (int row = 0; row < m_uavTable->rowCount(); ++row) {
        QTableWidgetItem *idItem = m_uavTable->item(row, idCol);
        if (idItem && idItem->text().toInt() == id) {
            // 只更新位置相关列
            if (auto *itemX = m_uavTable->item(row, xCol)) {
                itemX->setText(QString::number(uav->getX(), 'f', 2));
            }
            if (auto *itemY = m_uavTable->item(row, yCol)) {
                itemY->setText(QString::number(uav->getY(), 'f', 2));
            }
            break;
        }
    }
}

void MainWindow::updateUavStatus(int uavId)
{
    if (!m_uavTable || !m_ppiItem)
        return;

    // 获取目标的当前状态
    QSharedPointer<Mubiao> mb = m_ppiItem->getMubiaoHash().value(uavId);
    if (!mb)
        return;

    // 动态获取列索引
    int idCol     = getColumnIndexByName("ID");
    int statusCol = getColumnIndexByName("Status");

    if (idCol < 0 || statusCol < 0)
        return;   // 列名不存在

    // 遍历表格查找对应的行（因为排序可能改变了行号）
    for (int row = 0; row < m_uavTable->rowCount(); ++row) {
        QTableWidgetItem *idItem = m_uavTable->item(row, idCol);
        if (idItem && idItem->text().toInt() == uavId) {
            // 找到对应行，更新状态（使用自定义排序表格项）
            SortableTableWidgetItem *statusItem = dynamic_cast<SortableTableWidgetItem *>(m_uavTable->item(row, statusCol));
            if (!statusItem) {
                statusItem = new SortableTableWidgetItem();
                m_uavTable->setItem(row, statusCol, statusItem);
            }

            // 根据状态设置文本和排序键（移除数字前缀，使用UserRole排序）
            if (mb->daoyin_flag) {
                statusItem->setText("Guidance");
                statusItem->setData(Qt::UserRole, 0);   // Guidance=0，排在最前
                statusItem->setForeground(QBrush(Qt::red));
            }
            else if (mb->zhongdian) {
                statusItem->setText("Priority");
                statusItem->setData(Qt::UserRole, 1);                     // Priority=1，排在中间
                statusItem->setForeground(QBrush(QColor(255, 165, 0)));   // 橙色
            }
            else {
                statusItem->setText("Normal");
                statusItem->setData(Qt::UserRole, 2);   // Normal=2，排在最后
                statusItem->setForeground(QBrush(Qt::black));
            }

            // 触发表格重新排序（动态获取当前排序列的索引，支持列拖动）
            int currentSortCol = getColumnIndexByName(m_sortColumnName);
            if (currentSortCol >= 0) {
                m_uavTable->sortItems(currentSortCol, Qt::AscendingOrder);
            }
            break;
        }
    }
}

void MainWindow::onUavClicked(int uavId)
{
    if (!m_simManager || !m_uavTable)
        return;

    // 切换重点关注状态
    if (m_ppiItem) {
        m_ppiItem->toggleTargetFocus(uavId);
    }

    // 动态获取ID列索引
    int idCol = getColumnIndexByName("ID");
    if (idCol < 0)
        return;   // ID列不存在

    // 高亮表格中对应行（需要遍历查找，因为排序改变了行号）
    for (int row = 0; row < m_uavTable->rowCount(); ++row) {
        QTableWidgetItem *idItem = m_uavTable->item(row, idCol);
        if (idItem && idItem->text().toInt() == uavId) {
            m_uavTable->selectRow(row);
            m_uavTable->scrollToItem(idItem);
            break;
        }
    }

    // 查找对应的 UavModel，更新目标详情和标签信息
    const auto &uavs = m_simManager->getUavs();
    for (const auto &uavPtr : uavs) {
        if (uavPtr->getId() == uavId) {
            // 检查目标的当前状态（是否在导引中）
            QSharedPointer<Mubiao> mb        = m_ppiItem->getMubiaoHash().value(uavId);
            bool                   isGuiding = mb && mb->daoyin_flag;

            // 更新目标详情显示
            if (m_targetDetailLabel) {
                if (isGuiding) {
                    // 如果正在导引，显示导引状态（保持红框样式）
                    QString guidanceInfo = QString("<div style='text-align: center;'>"
                                                   "<b>=========================</b><br>"
                                                   "<span style='color: red; font-size: 14pt;'><b>GUIDANCE INITIATED</b></span><br>"
                                                   "<b>=========================</b><br>"
                                                   "</div>"
                                                   "<br>"
                                                   "<b>Target ID:</b> %1<br>"
                                                   "<b>Name:</b> %2<br>"
                                                   "<b>Position:</b> (%3, %4)<br>"
                                                   "<b>Status:</b> <span style='color: red;'>★ TRACKING</span><br>"
                                                   "<br>"
                                                   "<div style='text-align: center;'>"
                                                   "<b>=========================</b><br>"
                                                   "<i>Double-click to cancel</i>"
                                                   "</div>")
                                               .arg(uavPtr->getId())
                                               .arg(uavPtr->getName())
                                               .arg(QString::number(uavPtr->getX(), 'f', 2))
                                               .arg(QString::number(uavPtr->getY(), 'f', 2));

                    m_targetDetailLabel->setStyleSheet("QLabel { "
                                                       "background-color: #ffe6e6; "
                                                       "padding: 10px; "
                                                       "border: 3px solid #ff0000; "
                                                       "border-radius: 5px; "
                                                       "font-weight: bold; "
                                                       "}");
                    m_targetDetailLabel->setText(guidanceInfo);
                }
                else {
                    // 非导引状态，显示普通信息（恢复默认样式）
                    QString details = QString("Target ID: %1\n"
                                              "Name: %2\n"
                                              "Position: (%3, %4)\n"
                                              "Click to toggle focus\n"
                                              "Double-click for guidance")
                                          .arg(uavPtr->getId())
                                          .arg(uavPtr->getName())
                                          .arg(QString::number(uavPtr->getX(), 'f', 2))
                                          .arg(QString::number(uavPtr->getY(), 'f', 2));
                    m_targetDetailLabel->setText(details);

                    // 恢复默认样式
                    m_targetDetailLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 8px; border: 1px solid #ccc; border-radius: 3px; }");
                }
            }

            // 更新标签下方的信息（如果有传统图元）
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

void MainWindow::onTargetFocusToggled(int targetId, bool focused)
{
    qDebug() << "Target" << targetId << (focused ? "marked as PRIORITY" : "unmarked from priority");

    // 更新表格状态列
    updateUavStatus(targetId);

    // 检查目标是否处于导引状态
    QSharedPointer<Mubiao> mb        = m_ppiItem->getMubiaoHash().value(targetId);
    bool                   isGuiding = mb && mb->daoyin_flag;

    // 动态获取ID列索引
    int idCol = getColumnIndexByName("ID");
    if (idCol < 0)
        return;   // ID列不存在

    // 如果目标正在导引，不更新详情标签（让导引样式保持）
    if (isGuiding) {
        // 只高亮表格行，不更新详情
        for (int row = 0; row < m_uavTable->rowCount(); ++row) {
            QTableWidgetItem *idItem = m_uavTable->item(row, idCol);
            if (idItem && idItem->text().toInt() == targetId) {
                m_uavTable->selectRow(row);
                m_uavTable->scrollToItem(idItem);
                break;
            }
        }
        return;
    }

    // 更新UI显示（仅当不在导引状态时）
    if (m_targetDetailLabel) {
        // 在详情中添加关注状态
        const auto &uavs = m_simManager->getUavs();
        for (const auto &uavPtr : uavs) {
            if (uavPtr->getId() == targetId) {
                QString details = QString("Target ID: %1\n"
                                          "Name: %2\n"
                                          "Position: (%3, %4)\n"
                                          "Status: %5\n"
                                          "Click to toggle focus\n"
                                          "Double-click for guidance")
                                      .arg(uavPtr->getId())
                                      .arg(uavPtr->getName())
                                      .arg(QString::number(uavPtr->getX(), 'f', 2))
                                      .arg(QString::number(uavPtr->getY(), 'f', 2))
                                      .arg(focused ? "PRIORITY" : "Regular");
                m_targetDetailLabel->setText(details);

                // 恢复默认样式
                m_targetDetailLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 5px; border: 1px solid #ccc; border-radius: 3px; }");
                break;
            }
        }
    }

    // 高亮表格中对应行（需要遍历查找，因为排序改变了行号）
    for (int row = 0; row < m_uavTable->rowCount(); ++row) {
        QTableWidgetItem *idItem = m_uavTable->item(row, idCol);
        if (idItem && idItem->text().toInt() == targetId) {
            m_uavTable->selectRow(row);
            m_uavTable->scrollToItem(idItem);
            break;
        }
    }
}

void MainWindow::onTargetDoubleClicked(int targetId)
{
    // 切换导引状态
    if (m_ppiItem) {
        m_ppiItem->toggleTargetGuidance(targetId);
    }
}

void MainWindow::onTargetGuidanceToggled(int targetId, bool guiding)
{
    if (guiding) {
        qDebug() << "Initiating GUIDANCE for Target" << targetId;
    }
    else {
        qDebug() << "Canceling GUIDANCE for Target" << targetId;
    }

    // 先更新表格状态列（这会触发排序）
    updateUavStatus(targetId);

    // 然后更新详情显示（确保在状态更新之后）
    const auto &uavs = m_simManager->getUavs();
    for (const auto &uavPtr : uavs) {
        if (uavPtr->getId() == targetId) {
            if (guiding) {
                // 导引中 - 使用HTML格式确保正确显示
                QString guidanceInfo = QString("<div style='text-align: center;'>"
                                               "<b>=========================</b><br>"
                                               "<span style='color: red; font-size: 14pt;'><b>GUIDANCE INITIATED</b></span><br>"
                                               "<b>=========================</b><br>"
                                               "</div>"
                                               "<br>"
                                               "<b>Target ID:</b> %1<br>"
                                               "<b>Name:</b> %2<br>"
                                               "<b>Position:</b> (%3, %4)<br>"
                                               "<b>Status:</b> <span style='color: red;'>★ TRACKING</span><br>"
                                               "<br>"
                                               "<div style='text-align: center;'>"
                                               "<b>=========================</b><br>"
                                               "<i>Double-click to cancel</i>"
                                               "</div>")
                                           .arg(uavPtr->getId())
                                           .arg(uavPtr->getName())
                                           .arg(QString::number(uavPtr->getX(), 'f', 2))
                                           .arg(QString::number(uavPtr->getY(), 'f', 2));

                m_targetDetailLabel->setStyleSheet("QLabel { "
                                                   "background-color: #ffe6e6; "
                                                   "padding: 10px; "
                                                   "border: 3px solid #ff0000; "
                                                   "border-radius: 5px; "
                                                   "font-weight: bold; "
                                                   "}");
                m_targetDetailLabel->setText(guidanceInfo);
            }
            else {
                // 取消导引
                QString guidanceInfo = QString("<b>Target ID:</b> %1<br>"
                                               "<b>Name:</b> %2<br>"
                                               "<b>Position:</b> (%3, %4)<br>"
                                               "<b>Status:</b> Guidance Canceled<br>"
                                               "<br>"
                                               "Click to toggle focus<br>"
                                               "Double-click for guidance")
                                           .arg(uavPtr->getId())
                                           .arg(uavPtr->getName())
                                           .arg(QString::number(uavPtr->getX(), 'f', 2))
                                           .arg(QString::number(uavPtr->getY(), 'f', 2));

                m_targetDetailLabel->setStyleSheet("QLabel { "
                                                   "background-color: #f0f0f0; "
                                                   "padding: 8px; "
                                                   "border: 1px solid #ccc; "
                                                   "border-radius: 3px; "
                                                   "}");
                m_targetDetailLabel->setText(guidanceInfo);
            }

            // 动态获取ID列索引
            int idCol = getColumnIndexByName("ID");
            if (idCol < 0)
                return;   // ID列不存在

            // 高亮对应表格行（需要遍历查找，因为排序改变了行号）
            for (int row = 0; row < m_uavTable->rowCount(); ++row) {
                QTableWidgetItem *idItem = m_uavTable->item(row, idCol);
                if (idItem && idItem->text().toInt() == targetId) {
                    m_uavTable->selectRow(row);
                    m_uavTable->scrollToItem(idItem);
                    break;
                }
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
    // 这样可以保证：拖动PPI时，PPI上的目标和航迹整体一起移动，而不是重新计算方位/距离导致"飘动"。
    m_ppiDataManager->updateFromUavs(m_simManager->getUavs(),
                                     0.0,   // 雷达中心X（数据坐标系）
                                     0.0,   // 雷达中心Y（数据坐标系）
                                     radius,
                                     huanJu);

    // 获取更新后的数据并同步到PPI图元
    LockedHash<Mubiao> &sourceData = m_ppiDataManager->getMubiaoHash();
    LockedHash<Mubiao> &targetData = m_ppiItem->getMubiaoHash();

    // 保留现有的状态标志（zhongdian、daoyin_flag、labelOffset等）
    // 不要直接清空，而是更新现有数据
    QList<int> sourceKeys = sourceData.keys();

    for (int id : sourceKeys) {
        QSharedPointer<Mubiao> sourceMubiao = sourceData.value(id);
        if (!sourceMubiao)
            continue;

        // 检查目标是否已存在
        QSharedPointer<Mubiao> targetMubiao = targetData.value(id);
        if (targetMubiao) {
            // 目标已存在，只更新位置和航迹数据，保留状态标志
            targetMubiao->fangwei = sourceMubiao->fangwei;
            targetMubiao->gaodi   = sourceMubiao->gaodi;
            targetMubiao->juli    = sourceMubiao->juli;
            targetMubiao->circleppi_hangji.copyFrom(sourceMubiao->circleppi_hangji);
            targetMubiao->new_time = sourceMubiao->new_time;
            // 不更新 zhongdian、daoyin_flag、labelOffset 等用户交互状态
        }
        else {
            // 新目标，直接插入
            targetData.insert(id, sourceMubiao);
        }
    }

    // 移除不存在的目标
    QList<int> targetKeys = targetData.keys();
    for (int id : targetKeys) {
        if (!sourceData.contains(id)) {
            targetData.remove(id);
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
        m_scene->setShowGrid(false);
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

    qDebug() << "=== Applying Radar-Only Mode ===";

    // 1. 将地图移出视野（而不是hide），避免QQuickWidget的显示问题
    if (m_mapWidget) {
        // 移动到屏幕外，保持存活状态
        m_mapWidget->setGeometry(-10000, -10000, 100, 100);
        qDebug() << "Map moved out of view";
    }

    // 2. 设置场景和视图背景为黑色（纯雷达模式）
    m_scene->setShowGrid(false);
    m_scene->setBackgroundBrush(QBrush(QColor(0, 0, 0)));   // 黑色背景
    m_view->setStyleSheet("background: black;");            // 视图背景黑色
    m_view->viewport()->setStyleSheet("background: black;");

    //     3. 调整PPI显示：不透明，显示背景，放大半径
    m_ppiItem->setRadius(450);            // 放大半径以填充更多视图
    m_ppiItem->setPPIOpacity(1.0);        // 完全不透明
    m_ppiItem->setDrawBackground(true);   // 绘制PPI背景
    m_ppiItem->setZValue(10);             // 保持在上层

    // 4. 隐藏传统图元
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

    // 5. 调整视图缩放和居中
    m_view->resetTransform();
    m_view->scale(1.0, 1.0);
    m_view->centerOn(0, 0);

    qDebug() << "Switched to Radar-Only Mode";
}

void MainWindow::applyMapRadarMode()
{
    if (!m_ppiItem || !m_scene || !m_view)
        return;

    qDebug() << "=== Applying Map+Radar Mode ===";

    // 2. 先设置场景和视图背景为透明（让地图透过来）
    m_scene->setBackgroundBrush(Qt::transparent);
    m_scene->setShowGrid(false);
    m_view->setStyleSheet("background: transparent;");
    m_view->viewport()->setStyleSheet("background: transparent;");

    // 3. 调整PPI显示：半透明，不显示背景，叠加在地图上
    m_ppiItem->setRadius(400);
    m_ppiItem->setPPIOpacity(0.7);
    m_ppiItem->setDrawBackground(false);
    m_ppiItem->setZValue(10);

    // 1. 恢复地图到正确位置和大小（关键！）
    if (m_mapWidget && m_stackWidget) {
        QSize stackSize = m_stackWidget->size();

        qDebug() << "Before restore - Map geometry:" << m_mapWidget->geometry();
        qDebug() << "Stack size:" << stackSize;

        // 移回正确位置
        m_mapWidget->setGeometry(0, 0, stackSize.width(), stackSize.height());

        // 确保可见
        m_mapWidget->setVisible(true);
        m_mapWidget->show();

        // 层叠顺序
        m_mapWidget->lower();

        qDebug() << "After restore - Map geometry:" << m_mapWidget->geometry();
        qDebug() << "Map visible:" << m_mapWidget->isVisible();

        // 强制QQuickWidget刷新
        m_mapWidget->quickWindow()->update();
        m_mapWidget->update();

        // 同步地图中心和数据
        QTimer::singleShot(100, this, [this]() {
            syncMapCenter();
            syncUavDataToMap();
        });
    }

    // 4. 确保视图在地图之上
    if (m_view && m_stackWidget) {
        QSize stackSize = m_stackWidget->size();
        m_view->setGeometry(0, 0, stackSize.width(), stackSize.height());
        m_view->setVisible(true);
        m_view->show();
        m_view->raise();
    }

    // 5. 隐藏传统图元
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

    // 6. 恢复视图缩放
    m_view->resetTransform();
    m_view->scale(1.0, 1.0);
    m_view->centerOn(0, 0);

    qDebug() << "=== Map+Radar Mode Applied ===";
}

void MainWindow::togglePPIDrag()
{
    if (!m_ppiItem)
        return;

    bool currentState = m_ppiItem->getDraggable();
    bool newState     = !currentState;

    m_ppiItem->setDraggable(newState);

    // 更新按钮文本和记录PPI位置
    if (newState) {
        m_dragButton->setText("Disable PPI Drag");
        // 记录当前PPI位置，用于后续计算偏移
        m_lastPPIPos = m_ppiItem->pos();
        qDebug() << "PPI Drag Enabled - Drag to move map around radar position";
    }
    else {
        m_dragButton->setText("Enable PPI Drag");
        // 禁用拖动时，将PPI恢复到原点
        m_ppiItem->setPos(0, 0);
        m_lastPPIPos = QPointF(0, 0);
        qDebug() << "PPI Drag Disabled - PPI reset to center";
    }
}

int MainWindow::getColumnIndexByName(const QString &columnName) const
{
    if (!m_uavTable)
        return -1;

    // 遍历所有列，查找匹配的列名
    for (int col = 0; col < m_uavTable->columnCount(); ++col) {
        QString headerText = m_uavTable->horizontalHeaderItem(col)->text();
        if (headerText == columnName) {
            return col;
        }
    }

    return -1;   // 未找到
}

void MainWindow::sceneToGeo(double sceneX, double sceneY, double &latitude, double &longitude) const
{
    // 简单线性映射：场景坐标(单位：米) -> 地理坐标(单位：度)
    // 场景原点(0,0) 对应参考点(m_refLatitude, m_refLongitude)
    // 注意：Y轴向北为正，X轴向东为正
    latitude  = m_refLatitude + (sceneY / m_metersPerDegreeLat);
    longitude = m_refLongitude + (sceneX / m_metersPerDegreeLon);
}

double MainWindow::calculateMapZoomLevel(double huanJuMeters) const
{
    // 根据PPI的距离环半径计算合适的地图缩放级别
    // OpenStreetMap缩放级别：每增加1级，地图缩放2倍
    // 在赤道上，zoom=0时，256像素代表整个地球周长40075km
    // zoom级别n时，每像素代表的距离 = 40075000 / (256 * 2^n) 米

    // 假设地图显示区域宽度为800像素，我们希望PPI的最大半径正好显示在这个范围内
    // 那么需要的缩放级别为：huanJuMeters * 4（因为PPI半径到直径，再留一些边距）

    // 经验公式：zoom = 16 - log2(huanJuMeters / 1000)
    // huanJuMeters = 1000米 -> zoom = 16
    // huanJuMeters = 2000米 -> zoom = 15
    // huanJuMeters = 500米 -> zoom = 17

    if (huanJuMeters <= 0) {
        return 14;   // 默认缩放级别
    }

    double zoom = 16.0 - std::log2(huanJuMeters / 1000.0);

    // 限制缩放范围在 10-18 之间
    if (zoom < 10)
        zoom = 10;
    if (zoom > 18)
        zoom = 18;

    return zoom;
}

void MainWindow::syncMapCenter()
{
    if (!m_mapWidget)
        return;

    // 将地图中心设置为PPI圆心（天安门）
    m_mapWidget->setCenter(m_refLatitude, m_refLongitude);

    // 根据当前PPI的距离环设置地图缩放级别
    if (m_ppiItem) {
        double huanJu    = m_ppiItem->getHuanJu();   // 获取当前距离环（米）
        double zoomLevel = calculateMapZoomLevel(huanJu);
        m_mapWidget->setZoomLevel(zoomLevel);
        m_ringDistanceEdit->setText(QString::number(huanJu));
        qDebug() << "Sync map center: HuanJu =" << huanJu << "m, Zoom level =" << zoomLevel;
    }
}

bool MainWindow::isUavInPPIRange(double uavX, double uavY) const
{
    if (!m_ppiItem)
        return false;

    // PPI圆心在场景原点(0, 0)
    // 计算UAV到圆心的距离
    double distance = std::sqrt(uavX * uavX + uavY * uavY);

    // 获取PPI的最大显示半径
    double ppiRadius = m_ppiItem->getRadius();   // 这是屏幕像素半径
    double huanJu    = m_ppiItem->getHuanJu();   // 这是实际距离（米）

    // PPI显示的最大距离是 radius / huanJu 倍的距离环
    // 通常PPI显示多圈距离环，假设显示3圈
    double maxDisplayDistance = huanJu * 3.0;   // 可以根据实际情况调整

    return distance <= maxDisplayDistance;
}

void MainWindow::syncUavDataToMap()
{
    if (!m_mapWidget)
        return;

    // 遍历所有UAV，根据距离判断是否显示在地图上
    for (const auto &uav : m_simManager->getUavs()) {
        int    uavId = uav->getId();
        double uavX  = uav->getX();
        double uavY  = uav->getY();

        // 判断UAV是否在PPI范围内
        bool inPPIRange = isUavInPPIRange(uavX, uavY);

        if (inPPIRange) {
            // 在PPI范围内，从地图上移除该标记（避免重复显示）
            m_mapWidget->removeUavMarker(uavId);
        }
        else {
            // 在PPI范围外，显示在地图上
            double latitude, longitude;
            sceneToGeo(uavX, uavY, latitude, longitude);

            // 添加或更新地图标记
            m_mapWidget->addOrUpdateUavMarker(uavId, latitude, longitude, uav->getName());

            // 根据UAV状态设置标记颜色
            QSharedPointer<Mubiao> mubiao = m_ppiDataManager->getMubiaoHash().value(uavId);
            if (mubiao) {
                QString color = "blue";   // 默认颜色
                if (mubiao->daoyin_flag) {
                    color = "red";   // 导引状态：红色
                }
                else if (mubiao->zhongdian) {
                    color = "orange";   // 重点关注：橙色
                }
                else {
                    color = "green";   // 正常状态：绿色
                }
                m_mapWidget->setMarkerColor(uavId, color);
            }
        }
    }
}

void MainWindow::onTableHeaderClicked(int logicalIndex)
{
    // 当用户点击表头排序时，记录当前排序的列名
    if (!m_uavTable || logicalIndex < 0 || logicalIndex >= m_uavTable->columnCount())
        return;

    QTableWidgetItem *headerItem = m_uavTable->horizontalHeaderItem(logicalIndex);
    if (headerItem) {
        m_sortColumnName = headerItem->text();
        qDebug() << "Sorting by column:" << m_sortColumnName;
    }
}

void MainWindow::onPPIPositionChanged()
{
    if (!m_ppiItem || !m_mapWidget || m_displayMode != DisplayMode::MapRadar)
        return;

    // 获取PPI当前位置（场景坐标）
    QPointF ppiPos = m_ppiItem->pos();

    // PPI的位置偏移代表"雷达中心"的移动
    // PPI从(0,0)移动到(ppiPos.x(), ppiPos.y())
    // 意味着雷达中心从天安门移动了这个距离

    // 将场景偏移转换为地理坐标
    double newLatitude, newLongitude;
    sceneToGeo(ppiPos.x(), ppiPos.y(), newLatitude, newLongitude);

    // 更新地图中心到新位置
    m_mapWidget->setCenter(newLatitude, newLongitude);

    qDebug() << "PPI dragged: offset =" << ppiPos << "-> Map center:" << newLatitude << "," << newLongitude;

    // 重新同步UAV数据到地图（因为地图中心变了）
    syncUavDataToMap();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    // 同步调整地图和视图的大小，确保它们始终保持一致并填充整个容器
    if (m_stackWidget && m_mapWidget && m_view) {
        // 使用 stackWidget 的实际大小
        QSize stackSize = m_stackWidget->size();

        // 确保地图和视图完全覆盖 stackWidget
        m_mapWidget->setGeometry(0, 0, stackSize.width(), stackSize.height());
        m_view->setGeometry(0, 0, stackSize.width(), stackSize.height());

        qDebug() << "Resized: Stack size =" << stackSize << "Map/View geometry updated";
    }
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    // 只在地图+雷达模式下处理滚轮缩放
    if (m_displayMode != DisplayMode::MapRadar || !m_ppiItem) {
        QMainWindow::wheelEvent(event);
        return;
    }

    // 获取滚轮的滚动方向和步数
    int delta = event->angleDelta().y();   // 正数表示向上滚（放大），负数表示向下滚（缩小）

    // 计算缩放步长
    double scaleFactor = 1.0;
    if (delta > 0) {
        scaleFactor = 0.8;   // 向上滚，缩小距离环（放大显示）
    }
    else if (delta < 0) {
        scaleFactor = 1.25;   // 向下滚，放大距离环（缩小显示）
    }

    // 调整PPI的距离环
    double currentHuanJu = m_ppiItem->getHuanJu();
    double newHuanJu     = currentHuanJu * scaleFactor;

    // 限制距离环范围（100米到10000米）
    if (newHuanJu < 100)
        newHuanJu = 100;
    if (newHuanJu > 10000)
        newHuanJu = 10000;

    // 设置新的距离环
    m_ppiItem->setHuanJu(newHuanJu);

    // 同步地图缩放级别
    syncMapCenter();

    // 重新同步UAV数据（因为PPI范围改变了）
    syncUavDataToPPI();
    syncUavDataToMap();

    qDebug() << "Wheel zoom: HuanJu changed from" << currentHuanJu << "to" << newHuanJu << "meters";

    event->accept();
}
