#include "MainWindow.h"
#include "trajectorygenerator.h"
#include <QGridLayout>
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
    // 1. 初始化 Core 模块的业务对象
    m_uav = std::make_unique<UavModel>(101, "Phantom-X");
    /**
        // 1. 初始化 Core 模块的业务对象
        m_uav = std::make_unique<UavModel>(101, "Phantom-X");
        // 2. 生成航迹数据 (圆形，圆心在 100,100，半径 50，共 60 个点)
        //    QVector<QPointF> circlePath = TrajectoryGenerator::createCirclePath(QPointF(100, 100), 50.0, 300);
        QVector<QPointF> circlePath = TrajectoryGenerator::createEightShapePath(QPointF(100, 100), 50.0, 300);

        // 3. 将航迹传给无人机
        m_uav->setFlightPath(circlePath);

        // 2. 设置简单的 UI (实际项目中通常使用 .ui 文件设计，这里为了演示纯代码构建)
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QVBoxLayout *layout = new QVBoxLayout(centralWidget);

        QLabel *infoLabel = new QLabel("Ready to fly...", this);
        btnMove           = new QPushButton("Next Step", this);

        layout->addWidget(infoLabel);
        layout->addWidget(btnMove);

        // 3. 连接信号与槽 (Connect Signal & Slot)
        // 当按钮被点击 (clicked) 时，执行 lambda 表达式中的代码
        // 这类似于 Java 的 btn.addActionListener(() -> { ... });
        connect(btnMove, &QPushButton::clicked, this, [this, infoLabel]() {
            // 调用 Core 模块的逻辑
            m_uav->updatePosition(m_currentStep);
            m_currentStep++;

            // 更新 UI
            QString status = QString("Step: %1 | Pos: (%2, %3)")
                                 .arg(m_currentStep)
                                 // QString::number(val, format, precision) 控制小数位数
                                 .arg(QString::number(m_uav->getX(), 'f', 2))
                                 .arg(QString::number(m_uav->getY(), 'f', 2));
            infoLabel->setText(status);
        });
        resize(400, 300);
    **/
    setupUI();
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
    // 1. core逻辑更新
    m_uav->updatePosition(m_currentStep);
    m_currentStep++;

    // 2. gui界面更新，setPos()负责移动QGraphicsItem,但不会触发重绘，因为Qt优化
    m_uavItem->setPos(m_uav->getX(), m_uav->getY());
    updateTelemetry();
    if (m_currentStep >= m_uav->getPath().size()) {
        m_currentStep = 0;   // 重置到起点
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
    //    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scene->setSceneRect(-2000, -2000, 4000, 4000);
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

    // --- 2b. 遥测数据显示 ---
    layout->addWidget(new QLabel("UAV ID:"), 1, 0);
    layout->addWidget(new QLabel(QString::number(m_uav->getId())), 1, 1);

    layout->addWidget(new QLabel("Current X:"), 2, 0);
    m_posLabel = new QLabel("N/A");   // 动态更新的坐标标签
    layout->addWidget(m_posLabel, 2, 1);

    layout->addWidget(new QLabel("Current Step:"), 3, 0);
    m_statusLabel = new QLabel("0");   // 动态更新的步数标签
    layout->addWidget(m_statusLabel, 3, 1);

    dockContents->setLayout(layout);
    controlDock->setWidget(dockContents);
    m_controlButton->setText("▶ Start Simulation");

    // 3. 航迹数据生成与图元创建
    QPointF center(100.0, 100.0);
    //    QVector<QPointF> CirclePath = TrajectoryGenerator::createCirclePath(center, 50.0, 60);
    QVector<QPointF> CirclePath = TrajectoryGenerator::createEightShapePath(center, 50.0, 60);
    m_uav->setFlightPath(CirclePath);

    // 创建PathItem并添加到场景
    m_pathItem = new PathItem(m_uav->getPath());
    m_scene->addItem(m_pathItem);

    // 创建UavItem 并添加到场景
    m_uavItem = new UavItem();
    m_scene->addItem(m_uavItem);

    // 初始化无人机在航迹的第一个点
    m_uav->updatePosition(0);
    // 关键：将QGraphicsItem 移动到Service逻辑的位置
    m_uavItem->setPos(m_uav->getX(), m_uav->getY());

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
    // 初始化遥测更新
    updateTelemetry();
    resize(500, 600);   // 调整窗口大小以容纳视图
}

void MainWindow::updateTelemetry()
{
    m_statusLabel->setText(QString::number(m_currentStep));
    QString posText = QString("(%1, %2)").arg(QString::number(m_uav->getX(), 'f', 2)).arg(QString::number(m_uav->getY(), 'f', 2));
    m_posLabel->setText(posText);
}
