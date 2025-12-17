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
        connect(m_ppiItem, &PPIGraphicsItem::targetClicked, this, &MainWindow::onUavClicked, Qt::QueuedConnection);
        connect(m_ppiItem, &PPIGraphicsItem::targetDoubleClicked, this, &MainWindow::onTargetDoubleClicked, Qt::QueuedConnection);
        connect(m_ppiItem, &PPIGraphicsItem::targetFocusToggled, this, &MainWindow::onTargetFocusToggled, Qt::QueuedConnection);
        connect(m_ppiItem, &PPIGraphicsItem::targetGuidanceToggled, this, &MainWindow::onTargetGuidanceToggled, Qt::QueuedConnection);
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
                statusItem->setText("★ Guidance");
                statusItem->setData(Qt::UserRole, 0);   // Guidance=0，排在最前
                statusItem->setForeground(QBrush(Qt::red));
            }
            else if (mb->zhongdian) {
                statusItem->setText("◆ Priority");
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
