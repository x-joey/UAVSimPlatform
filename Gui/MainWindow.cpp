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
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    updatePathTimer = new QTimer(this);
    connect(updatePathTimer, &QTimer::timeout, this, &MainWindow::updatePathTimeout, Qt::QueuedConnection);
    // 1. 初始化 Service 模块的业务对象
    m_simManager = new SimulationManager();
    setupUI();
    // 连接场景中点击无人机的信号
    connect(m_scene, &SimScene::uavClicked, this, &MainWindow::onUavClicked);
    updatePathTimer->start(1000);
}

MainWindow::~MainWindow()
{
    // 这里的 m_uav 会被 std::unique_ptr 自动释放，不需要手动 delete
    // QGraphicsView/Scene都会被其parent自动释放，无需手动delete
    // 只有非QObject的指针或裸指针才需要手动delete
}

void MainWindow::updatePathTimeout()
{
    for (const auto &uav : m_simManager->getUavs()) {
        uav->updatePosition(m_currentStep);
        m_currentStep = (m_currentStep + 1) % uav->getPath().size();
        uav->setCurrentPoint(m_currentStep);
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
        qDebug() << "xujunwei:" << uav.get()->getId() << "," << m_currentStep << endl;
        if (uav->getCurrentPoint() >= uav->getPath().size()) {
            uav->setCurrentPoint(0);   // 重置到起点
        }
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

    // 设置场景的边界（坐标范围），根据你的航迹数据设定，保证内容在区域内
    // 我们之前设置圆心100，100，半径50.设置0，0到200，200足够显示
    //    m_scene->setSceneRect(0, 0, 200, 200);
    // 禁用view的滚动条，让它看起来更像一个固定画布
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scene->setSceneRect(-20000, -20000, 40000, 40000);
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);                 // 启用鼠标拖拽平移
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);   // 设置缩放锚点为“鼠标下方”
    m_view->setRenderHint(QPainter::Antialiasing);                      // 开启抗锯齿，让圆和线更平滑，设置渲染质量

    // --- 2. 遥测控制台 (QDockWidget) Setup ---
    QDockWidget *controlDock = new QDockWidget("Simulation Control", this);
    controlDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, controlDock);   // 停靠在右侧
    controlDock->setWindowFlags(Qt::FramelessWindowHint);

    // 去掉返回栏
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

    // --- 2b. 多架无人机遥测数据显示表格 ---
    m_uavTable = new QTableWidget(dockContents);
    initUavTable();
    layout->addWidget(new QLabel("UAV Telemetry:"), 1, 0, 1, 2);
    layout->addWidget(m_uavTable, 2, 0, 1, 2);

    dockContents->setLayout(layout);
    controlDock->setWidget(dockContents);
    m_controlButton->setText("▶ Start Simulation");

    // 3. 航迹数据生成与图元创建
    //    QPointF center(100.0, 100.0);
    //    //    QVector<QPointF> CirclePath = TrajectoryGenerator::createCirclePath(center, 50.0, 60);
    //    QVector<QPointF> CirclePath = TrajectoryGenerator::createEightShapePath(center, 50.0, 60);
    //    m_uav->setFlightPath(CirclePath);



    // 3. 布局与交互
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *vlayout   = new QVBoxLayout(centralWidget);
    QLabel      *infoLabel = new QLabel("Ready to fly...", this);
    //    btnMove                = new QPushButton("next step", this);

    vlayout->addWidget(m_view);
    vlayout->addWidget(infoLabel);
    //    vlayout->addWidget(btnMove);
    connect(m_controlButton, &QPushButton::clicked, this, &MainWindow::toggleSimulation);

    for (const auto &uav : m_simManager->getUavs()) {
        // 创建PathItem并添加到场景
        m_pathItem = new PathItem(uav->getPath());
        m_scene->addItem(m_pathItem);

        // 创建UavItem 并添加到场景（目标点）
        auto *uavItem = new UavItem();
        uavItem->setId(uav->getId());
        m_uavItemMap[uav->getId()] = uavItem;
        m_scene->addItem(uavItem);
        uavItem->setPos(uav->getX(), uav->getY());

        // 创建标签图元，并与无人机通过虚线连接
        auto *labelItem = new UavLabelItem();
        labelItem->setId(uav->getId());
        labelItem->setName(uav->getName());
        labelItem->setUavScenePos(QPointF(uav->getX(), uav->getY()));
        m_uavLabelMap[uav->getId()] = labelItem;
        m_scene->addItem(labelItem);

        // 初始化遥测、表格和标签信息
        updateTelemetry(uav.get());
        updateUavRow(uav.get());
        updateLabelInfo(uav.get());
    }

    resize(1000, 600);   // 调整窗口大小以容纳视图
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
