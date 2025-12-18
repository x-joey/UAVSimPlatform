#include "mainwindow.h"
#include "circleppiwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setupUI();
    setupConnections();
}

void MainWindow::setupUI()
{
    // 创建中心部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);  // 设置边距
    mainLayout->setSpacing(10);

    // 创建控制面板
    QHBoxLayout *controlLayout = new QHBoxLayout();

    QCheckBox *kuangXuanCheck = new QCheckBox("框选模式", this);
    QCheckBox *dianJiCheck = new QCheckBox("显示点迹", this);
    m_statusLabel = new QLabel("状态: 正常", this);

    controlLayout->addWidget(kuangXuanCheck);
    controlLayout->addWidget(dianJiCheck);
    controlLayout->addWidget(m_statusLabel);
    controlLayout->addStretch();

    // 创建PPI组件
    m_ppiWidget = new CirclePPIWidget(this);
    m_ppiWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 添加到主布局
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(m_ppiWidget);

    // 连接信号
    connect(kuangXuanCheck, &QCheckBox::toggled, this, &MainWindow::onToggleKuangXuan);
    connect(dianJiCheck, &QCheckBox::toggled, this, &MainWindow::onShowDianJi);
    connect(m_ppiWidget, &CirclePPIWidget::create_jinshe, this, &MainWindow::onCreateJinshe);

    setWindowTitle("Circle PPI 测试程序");
    resize(1400, 800);
}

void MainWindow::setupConnections()
{
    // 可以在这里添加其他连接
}

void MainWindow::onToggleKuangXuan(bool checked)
{
    m_ppiWidget->m_isSelecting = checked;
    m_statusLabel->setText(checked ? "模式: 框选" : "模式: 正常");
}

void MainWindow::onShowDianJi(bool checked)
{
    show_dianji = checked;
}

void MainWindow::onCreateJinshe(double left, double right, double top, double bottom)
{
    qDebug() << "创建禁射区域: 左" << left << "右" << right << "上" << top << "下" << bottom;
}
