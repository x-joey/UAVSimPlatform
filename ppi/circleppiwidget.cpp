#include "circleppiwidget.h"
#include <QPaintEvent>
#include <QtConcurrent>
#include <QApplication>
#include <QMutex>
#include <QThread>
#include <pthread.h>
#include <sched.h>

CirclePPIWidget::CirclePPIWidget(QWidget* parent) : QWidget{ parent }
{
    setWindowFlags(Qt::FramelessWindowHint);
    // 设置窗口背景颜色
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);

    resize(1400, 760);

    BeiXiang.load(":/image/image/BeiXiang.png");

    // 动态创建 ptn_jubiaojian
    ptn_jubiaojian = new QPushButton(this);
    ptn_jubiaojian->setText("-");
    ptn_jubiaojian->setGeometry(10, 10, 28, 28);  // 设置位置和大小
    connect(ptn_jubiaojian, &QPushButton::clicked, this, &CirclePPIWidget::JuBiaoJian_clicked);

    // 动态创建 lbl_jubiao
    lbl_jubiao = new QLabel(this);
    lbl_jubiao->setText("5km");
    lbl_jubiao->setGeometry(40, 10, 50, 28);  // 设置位置和大小
    // 设置文字颜色为白色
    QPalette palette = lbl_jubiao->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    lbl_jubiao->setPalette(palette);
    lbl_jubiao->setAlignment(Qt::AlignCenter);

    // 动态创建 ptn_jubiaojia
    ptn_jubiaojia = new QPushButton(this);
    ptn_jubiaojia->setText("+");
    ptn_jubiaojia->setGeometry(92, 10, 28, 28);  // 设置位置和大小
    connect(ptn_jubiaojia, &QPushButton::clicked, this, &CirclePPIWidget::JuBiaoJia_clicked);

    // 动态创建 ptn_fuwei
    ptn_fuwei = new QPushButton(this);
    ptn_fuwei->setText("复位");
    ptn_fuwei->setGeometry(10, 45, 110, 28);  // 设置位置和大小
    connect(ptn_fuwei, &QPushButton::clicked, this, &CirclePPIWidget::PPIGuiLing_clicked);

    mubiaoxingzhi = new MuBiao_XingZhi(this);
    mubiaoxingzhi->move(500, 200);
    mubiaoxingzhi->hide();

    m_isSelecting = false;

    m_backBuffer = QPixmap(size());
    m_backBuffer.fill(Qt::transparent);

    m_rulerBuffer = QPixmap(size());
    m_rulerBuffer.fill(Qt::transparent);

    // 新增多线程相关初始化
    m_renderThread = new QThread(this);
    m_frontBuffer  = QPixmap(size());

        m_clickTimer.setSingleShot(true);
        connect(&m_clickTimer, &QTimer::timeout, this, &CirclePPIWidget::mouseclick);  // 修改连接方式

#ifdef LINUX
    // 将渲染逻辑移到线程
    connect(m_renderThread, &QThread::started, [this]() {
        // 银河麒麟V10 CPU绑定
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(2, &cpuset);  // 绑定到第3个CPU核心
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    });
#endif

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &CirclePPIWidget::asyncRender);
    timer->start(200);

    m_renderThread->start();

    ppizhongxinx = width() / 2;
    ppizhongxiny = height() / 2;

}

CirclePPIWidget::~CirclePPIWidget()
{
    timer->stop();
    m_renderThread->quit();
    m_renderThread->wait();
}

// 新增异步渲染方法
void CirclePPIWidget::asyncRender()
{
    generateTestTargets();
    QMutexLocker locker(&m_bufferMutex);

    time_count++;

    // 在后台缓冲区绘制
    QPainter backBufferPainter(&m_backBuffer);

    backBufferPainter.setRenderHint(QPainter::Antialiasing, true);
    backBufferPainter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    backBufferPainter.fillRect(m_backBuffer.rect(), Qt::black);  // 填充背景

    // 缓存不常变化的计算结果
    const int arrowLength   = m_radius;
    const int arrowHeadSize = 8;

    if (m_isSelecting)
    {
        drawSelectionRect(backBufferPainter);
    }

    // 绘制火力线
    backBufferPainter.translate(ppizhongxinx, ppizhongxiny);
    backBufferPainter.rotate(HuoLiXian_MaPan);

    QPen linePen(QColor(0, 255, 234, 170), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    backBufferPainter.setPen(linePen);
    backBufferPainter.setBrush(QColor(0, 255, 234));

    const QPointF arrowEnd(0, -arrowLength);
    backBufferPainter.drawLine(QPointF(0, 0), arrowEnd);

    QPolygonF arrowHead;
    arrowHead << arrowEnd << QPointF(arrowEnd.x() - arrowHeadSize, arrowEnd.y() + arrowHeadSize)
              << QPointF(arrowEnd.x() + arrowHeadSize, arrowEnd.y() + arrowHeadSize);
    backBufferPainter.drawPolygon(arrowHead);

    // 恢复变换
    backBufferPainter.resetTransform();

    if (m_needRedrawRuler)
    {
        // 重新绘制标尺到缓存中
        m_rulerBuffer.fill(Qt::transparent);
        QPainter rulerPainter(&m_rulerBuffer);
        rulerPainter.fillRect(m_rulerBuffer.rect(), Qt::transparent);
        // 新增渲染提示设置
        rulerPainter.setRenderHint(QPainter::Antialiasing, true);
        rulerPainter.setRenderHint(QPainter::SmoothPixmapTransform, true);

        // 绘制北向
        rulerPainter.drawPixmap(ppizhongxinx + 5, ppizhongxiny - m_radius - 32, BeiXiang);
        paintruler(&rulerPainter);
        m_needRedrawRuler = false;
    }

    // 将缓存的标尺绘制到后台缓冲区
    backBufferPainter.drawPixmap(0, 0, m_rulerBuffer);

    //    if ((U209_SouSuoLeiDaGongZuoZhuangTai.GongZuoMoShiZhuangTaiHuiGao == 15)
    //        || (U209_SouSuoLeiDaGongZuoZhuangTai.GongZuoMoShiZhuangTaiHuiGao == 11))
    //    {
    //        double startAngle, endAngle;
    //        if (U209_SouSuoLeiDaGongZuoZhuangTai.GongZuoMoShiZhuangTaiHuiGao == 11)
    //        {
    //            startAngle = HuoLiXian_MaPan - 45;
    //            endAngle   = HuoLiXian_MaPan + 45;
    //        }
    //        else
    //        {
    //            startAngle = (zhongxinfw * 360 / 6000) - 45;
    //            endAngle   = (zhongxinfw * 360 / 6000) + 45;
    //        }
    //        GradientArc(4, &backBufferPainter, startAngle, endAngle);
    //    }

    // 绘制禁射扇形区域
    foreach (const JinSheQuYu& item, JSQY_list)
    {
        GradientArc(1, &backBufferPainter, item.ZuoBianJie, item.YouBianJie);
    }
    foreach (const JinSheQuYu& item, FX_JS_list)
    {
        GradientArc(1, &backBufferPainter, item.ZuoBianJie, item.YouBianJie);
    }
    foreach (const JinSheQuYu& item, QY_JS_list)
    {
        GradientArc(1, &backBufferPainter, item.ZuoBianJie, item.YouBianJie);
    }

    // 画干扰屏蔽区
    if (SS_ganraoqu1)
    {
        GradientArc(2, &backBufferPainter, SS_ganraoqu1_qishi, SS_ganraoqu1_jiesu);
    }
    if (SS_ganraoqu2)
    {
        GradientArc(2, &backBufferPainter, SS_ganraoqu2_qishi, SS_ganraoqu2_jiesu);
    }
    if (SS_ganraoqu3)
    {
        GradientArc(2, &backBufferPainter, SS_ganraoqu3_qishi, SS_ganraoqu3_jiesu);
    }
    // 静默扇区
    if (SS_jingmoqu1)
    {
        GradientArc(3, &backBufferPainter, SS_jingmoqu1_qishi, SS_jingmoqu1_jiesu);
    }
    if (SS_jingmoqu2)
    {
        GradientArc(3, &backBufferPainter, SS_jingmoqu2_qishi, SS_jingmoqu2_jiesu);
    }
    if (SS_jingmoqu3)
    {
        GradientArc(3, &backBufferPainter, SS_jingmoqu3_qishi, SS_jingmoqu3_jiesu);
    }

    if (Is_ZeRenShanQu_Use)
    {
        GradientArc(4, &backBufferPainter, mil_du(BenDiZeRenShanQu.sq_start), mil_du(BenDiZeRenShanQu.sq_end));
    }

    // 跟雷探测跟踪模式下，辐射后，绘制跟雷辐射范围
    if (U215_GenZongLeiDaGongZuoZhuangTai.GongZuoMoShiZhuangTaiHuiGao == 0
        && U215_GenZongLeiDaGongZuoZhuangTai.GenZongLeiDaFaSheGongZuoZhuangTai == 2)
    {
        if (U215_GenZongLeiDaGongZuoZhuangTai.GongZuoMoShiZhuangTaiHuiGao < 2
            && U215_GenZongLeiDaGongZuoZhuangTai.QuYuTanCeMoShiHuiGao == 0)
        {
            // 12°
            GradientArc(5, &backBufferPainter, HuoLiXian_MaPan - 6, HuoLiXian_MaPan + 6);
        }
        else if (U215_GenZongLeiDaGongZuoZhuangTai.GongZuoMoShiZhuangTaiHuiGao < 2
                 && U215_GenZongLeiDaGongZuoZhuangTai.QuYuTanCeMoShiHuiGao == 1)
        {
            // 40°
            GradientArc(5, &backBufferPainter, HuoLiXian_MaPan - 20, HuoLiXian_MaPan + 20);
        }
        else
        {
            ;  // 不做处理
        }
    }

    // 微波发射状态为发射中时，绘制波束导引信息
    if (C99_WeiBoXiTongGongZuoZhuangTaiHuiGao.B0.WeiBoFaSheZhuangTai == 1)
    {
        foreach (const UDP_242_MuBiao& item, U242_BoShuDaoYinXinXi.MuBiao_List)
        {
            drawDashedLine(&backBufferPainter, mil_du(item.FangWeiJiao * 6000 / 65536));
        }
    }

    //    QVector<int> copyVector(paixu_jieguo.get_data());
    auto         rhkq_copy  = rhkq.keys();  // 获取副本
    QVector<int> copyVector = QVector<int>::fromList(rhkq_copy);
    for (int index = 0; index < copyVector.size(); index++)
    {
        //        if (index > max_num)
        //        {
        //            // 限制最大目标个数为120
        //            continue;
        //        }

        int pihao = copyVector.at(index);

        QSharedPointer<Mubiao> mb = rhkq.value(pihao);
        if (mb)
        {
            //            bool isInRange = (qAbs(mb->fangwei - FangWei_44) <= 8) && (qAbs(mb->gaodi - GaoDi_44) <= 8);
            //            rhkq.modify(i, [&](Mubiao& mb) { mb.genzhong = isInRange; });
            if (! mb->daji_flag)
            {
                // 只绘制未打击目标
                draw_mubiao(mb, &backBufferPainter);
            }

            if (mb->zhongdian)
            {
                QSharedPointer<MBWigt_BP> item;

                // 标牌列表中不存在该批号的标牌，则新建
                if (! Widg_bp_list.contains(pihao))
                {
                    item = QSharedPointer<MBWigt_BP>::create(this);
                    item->show();
                    Widg_bp_list.insert(pihao, item);
                }
                else
                {
                    item = Widg_bp_list.value(pihao);
                }

                if (item)
                {
                    if (! mb->BiaoPai_move && !mb->circleppi_hangji.isEmpty())
                    {
                        const QPoint newPos(mb->circleppi_hangji.last().x() + 45, mb->circleppi_hangji.last().y() + 5);

                        rhkq.modify(pihao, [&](Mubiao& mb) { mb.point_biaopai = newPos; });
                        item->move(newPos);
                    }
                    item->set_params(mb->pihao);

                    if(!mb->circleppi_hangji.isEmpty()){
                    backBufferPainter.setPen(QColor(255, 255, 255));
                    backBufferPainter.drawLine(mb->circleppi_hangji.last().x(), mb->circleppi_hangji.last().y(),
                                               item->pos().x(), item->pos().y());}
                }
            }
            else
            {
                auto bp = Widg_bp_list.value(pihao);
                if (bp)
                {
                    rhkq.modify(pihao, [&](Mubiao& mb) { mb.BiaoPai_move = false; });
                    bp->hide();
                    Widg_bp_list.remove(pihao);
                }
            }
        }
    }

    if (time_count % 5 == 0)
    {
        // 1s周期性检测标牌对应目标是否消失
        QList<int> keys = Widg_bp_list.keys();
        for (int i = 0; i < keys.size(); ++i)
        {
            int pihao = keys.at(i);
            if (auto mb = rhkq.value(pihao))
            {
                // 删除已打击目标的标牌
                if (mb->daji_flag)
                {
                    auto bp = Widg_bp_list.value(pihao);
                    if (bp)
                    {
                        bp->hide();
                        Widg_bp_list.remove(pihao);
                    }
                }
            }
            else
            {
                // 删除已消失目标标牌
                auto bp = Widg_bp_list.value(pihao);
                if (bp)
                {
                    bp->hide();
                    Widg_bp_list.remove(pihao);
                }
            }
        }
        time_count = 0;
    }

    // 显示搜索雷达原始点迹数据
//    if (show_dianji && dj_point.size() > 0)
//    {
//        QVector<QVector<QPointF>> copyVector(dj_point.get_data());
//        const int                 afterglow = 255 / copyVector.size();
//        for (int i = 0; i < copyVector.size(); i++)
//        {
//            // 靠前的点时间长，越暗，靠后的点时间短，越亮
//            const QColor pointColor = QColor::fromRgb(qRgb(i * afterglow, i * afterglow, i * afterglow));
//            backBufferPainter.setPen(QPen(pointColor, 3));

//            QVector<QPointF> currentDjPoints = copyVector.at(i);
//            QPolygonF        polygon(currentDjPoints);
//            backBufferPainter.drawPoints(polygon);
//        }
//    }
    // 显示搜索雷达原始点迹数据
    if (show_dianji && dj_point.size() > 0)
    {
        QVector<QVector<QPointF>> copyVector = dj_point.get_data();

        // 调试输出
        static int dianjiDebugCount = 0;
        if (dianjiDebugCount++ % 50 == 0) {
            qDebug() << "绘制点迹: 组数 =" << copyVector.size()
                     << "show_dianji =" << show_dianji;
        }

        // 设置点迹绘制样式
        backBufferPainter.setRenderHint(QPainter::Antialiasing, true);

        for (int i = 0; i < copyVector.size(); i++)
        {
            QVector<QPointF> currentDjPoints = copyVector.at(i);

            // 设置点迹颜色 - 使用不同颜色区分不同组
            int colorValue = (i * 50) % 255;
            QColor pointColor = QColor::fromRgb(
                (colorValue + 100) % 255,
                (colorValue + 50) % 255,
                colorValue
            );

            QPen pointPen(pointColor, 4); // 增大点迹大小
            pointPen.setCapStyle(Qt::RoundCap);
            backBufferPainter.setPen(pointPen);

            // 绘制点迹
            for (const QPointF& point : currentDjPoints) {
                backBufferPainter.drawPoint(point);

                // 调试输出前几个点的位置
                if (dianjiDebugCount % 50 == 0 && i < 3 && currentDjPoints.indexOf(point) == 0) {
                    qDebug() << "点迹" << i << "位置: (" << point.x() << "," << point.y() << ")";
                }
            }
        }
    }
    // 交换缓冲区
    qSwap(m_frontBuffer, m_backBuffer);

    QMetaObject::invokeMethod(this, "update");  // 通知主线程更新
}

void CirclePPIWidget::mouseclick()
{
    qDebug() << "mouseclick";
    mubiaoxingzhi->hide();

    int pihao = get_Current_MuBiao(m_FirCliPos);

    if (pihao > 0)
    {
        // 点选位置的目标存在则执行重点关注操作
        if (rhkq.contains(pihao))
        {
            zhongdian_guanzhu_fun(pihao);
            LanJiePaiXu();
        }
        return;
    }
}

void CirclePPIWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    QMutexLocker locker(&m_bufferMutex);
    if (! m_frontBuffer.isNull())
    {
        painter.drawPixmap(0, 0, m_frontBuffer);
    }
}

// 当需要重新绘制标尺时，调用此函数设置标志位
void CirclePPIWidget::setNeedRedrawRuler(bool needRedraw)
{
    m_needRedrawRuler = needRedraw;
    if (needRedraw)
    {
        asyncRender();
    }
}

void CirclePPIWidget::draw_mubiao(QSharedPointer<Mubiao> mubiao, QPainter* painter)
{
    if (! painter || ! painter->isActive() || mubiao.isNull())
    {
        return;  // 增加绘制前校验
    }

    QVector<QPointF> copiedVector(mubiao->circleppi_hangji.get_data());
    // 添加安全检查
    if (copiedVector.isEmpty()) {
        static int emptyCount = 0;
        emptyCount++;
        if (emptyCount % 100 == 0) { // 每100次空绘制输出一次日志，避免日志过多
            qDebug() << "目标" << mubiao->pihao << "的copiedVector为空，位置计算可能有问题";
            qDebug() << "目标参数 - 方位:" << mubiao->fangwei << "距离:" << mubiao->juli;
            qDebug() << "PPI中心: (" << ppizhongxinx << "," << ppizhongxiny << ") 半径:" << m_radius;
        }
        return;
    }
    painter->save();
    QPointF pos = copiedVector.last();
    mubiao->draw_mubiao(painter, pos);

    // 计算目标航迹并绘制
    QPolygonF polygon(copiedVector);
    painter->drawPoints(polygon);
    painter->restore();
}

QPointF CirclePPIWidget::get_xyz(float fw, float gd, uint32_t jl)
{
    uint16_t huitu_juli = (uint16_t) (jl);

    if (huitu_juli > (5 * huan_ju))  // 如果距离超出显示最远距离，绘图距离为显示最远距离
    {
        huitu_juli = (uint16_t) (5 * huan_ju);
    }

    float x = ppizhongxinx + sin(fw * PI / 3000.0) * huitu_juli * m_radius * 0.2 / huan_ju;
    float y = ppizhongxiny - cos(fw * PI / 3000.0) * huitu_juli * m_radius * 0.2 / huan_ju;

    return QPointF(x, y);
}

void CirclePPIWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    // 调整PPI中心到新窗口的中心
    ppizhongxinx = width() / 2;
    ppizhongxiny = height() / 2;
    QMutexLocker locker(&m_bufferMutex);
    m_frontBuffer = QPixmap(size());
    m_backBuffer  = QPixmap(size());
}

void CirclePPIWidget::mousePressEvent(QMouseEvent* event)
{
    // 仅处理左键的延迟点击
    if (event->button() == Qt::LeftButton)
    {
        m_FirCliPos = event->pos();

        // 点选禁射
        if (is_dianxuan_jinshe)
        {
            if (left_dianxuan_jinshe == 0)
            {
                left_dianxuan_jinshe = mil_du(calc_zhongxinfw(m_FirCliPos));
            }
            else
            {
                right_dianxuan_jinshe = mil_du(calc_zhongxinfw(m_FirCliPos));
                is_dianxuan_jinshe    = false;
                emit create_jinshe(left_dianxuan_jinshe, right_dianxuan_jinshe, 90, -5);
                left_dianxuan_jinshe  = 0;
                right_dianxuan_jinshe = 0;
            }
            return;
        }

        // 点选责任扇区
        if (is_dianxuan_zerenshanqu)
        {
            if (left_dianxuan_jinshe == 0)
            {
                left_dianxuan_jinshe = calc_zhongxinfw(m_FirCliPos);
            }
            else
            {
                right_dianxuan_jinshe     = calc_zhongxinfw(m_FirCliPos);
                is_dianxuan_zerenshanqu   = false;
                BenDiZeRenShanQu.sq_start = left_dianxuan_jinshe;
                BenDiZeRenShanQu.sq_end   = right_dianxuan_jinshe;
                qDebug() << left_dianxuan_jinshe;
                qDebug() << right_dianxuan_jinshe;
                BenDiZeRenShanQu.sq_xia   = du_mil(-5);
                BenDiZeRenShanQu.sq_shang = du_mil(90);
                Is_ZeRenShanQu_Use        = true;
                left_dianxuan_jinshe      = 0;
                right_dianxuan_jinshe     = 0;
            }
            return;
        }

        // 框选
        if (m_isSelecting)
        {
            m_startPoint = m_FirCliPos;
            m_endPoint   = m_FirCliPos;
        }
        else
        {
            // 触发重点关注处理 启动计时器（等待双击间隔时间）,以免想要双击结果点慢了
            // 如果只是想单击，那么也会在timeout后触发mouseCLick
            m_clickTimer.start(QApplication::doubleClickInterval());
        }
    }

    if (event->button() == Qt::RightButton)
    {
        // 右键立即处理
        m_FirCliPos = event->pos();

        int pihao = get_Current_MuBiao(m_FirCliPos);
        if (event->button() == Qt::RightButton)
        {
            if (pihao > 0)
            {
                mubiaoxingzhi->move(m_FirCliPos.x(), m_FirCliPos.y());
                mubiaoxingzhi->m_pihao = pihao;
                if (mubiaoxingzhi->m_pihao != 0)
                {
                    mubiaoxingzhi->refresh();
                    mubiaoxingzhi->raise();
                    mubiaoxingzhi->show();
                }
                else
                {
                    mubiaoxingzhi->hide();
                }
            }
        }
    }
}

void CirclePPIWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    qDebug() << "mouseDoubleClickEvent";

    m_clickTimer.stop();

    m_FirCliPos = event->pos();

    int pihao = get_Current_MuBiao(m_FirCliPos);
    if (pihao > 0)
    {
        QSharedPointer<Mubiao> mb = rhkq.value(pihao);
        if (mb && ! (mb->zhongdian))
        {
            zhongdian_guanzhu_fun(pihao);
            LanJiePaiXu();
        }
        daoyin_fun(pihao);
    }
}

void CirclePPIWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_isSelecting)
    {
        if (event->button() == Qt::LeftButton)
        {
            if (m_startPoint != m_endPoint)
            {
                // 创建选择区域矩形（标准化坐标）
                QRectF selectArea = QRectF(m_startPoint, m_endPoint).normalized();

                // 清空之前的选中结果
                m_selectedPihao.clear();

                {
                    // 遍历所有目标检测是否在选择区域内
                    QList<int> keyList = rhkq.keys();
                    for (int index = 0; index < keyList.size(); index++)
                    {
                        int                    i   = keyList.at(index);
                        QSharedPointer<Mubiao> mb  = rhkq.value(i);
                        QPointF                pos = get_xyz(mb->fangwei, mb->gaodi, mb->juli);
                        if (mb && selectArea.contains(pos))
                        {
                            // 框选目标中非重点目标则重点关注
                            if (! mb->zhongdian)
                            {
                                m_selectedPihao.append(mb->pihao);
                            }
                        }
                    }

                    zhongdian_guanzhu_fun(m_selectedPihao);
                    LanJiePaiXu();
                }
            }

            // 重置起始点和结束点，使红色矩形消失
            m_startPoint = QPoint();
            m_endPoint   = QPoint();
            asyncRender();
        }
    }
    else
    {
        if (m_move == true)
        {
            if (event->button() == Qt::LeftButton)
            {
                ppizhongxinx += m_LosCliPos.x();
                ppizhongxiny += m_LosCliPos.y();
                QingChu_WeiJi();
                setNeedRedrawRuler(true);
            }
        }
        m_move = false;
    }
}

void CirclePPIWidget::mouseMoveEvent(QMouseEvent* event)
{
    // 只有在左键时才处理
    if (event->buttons() & Qt::LeftButton)
    {
        if (m_isSelecting)
        {
            // 框选
            m_endPoint = event->pos();
            asyncRender();
        }
        else
        {
            // 拖动PPI中心
            if (event->pos() != m_FirCliPos)
            {
                m_LosCliPos = event->pos() - m_FirCliPos;
                m_move      = true;
                setNeedRedrawRuler(true);
                m_clickTimer.stop();
            }
        }
    }
}

void CirclePPIWidget::wheelEvent(QWheelEvent* event)
{
    QPoint numDegrees = event->angleDelta();
    if (numDegrees.y() > 0)
    {
        if (m_radius >= 3000)
        {
            m_radius = 3000;
        }
        else
        {
            m_radius += 20;
        }
    }
    else
    {
        if (m_radius <= 300)
        {
            m_radius = 300;
        }
        else
        {
            m_radius -= 20;
        }
    }
    QingChu_WeiJi();

    setNeedRedrawRuler(true);
}

void CirclePPIWidget::QingChu_WeiJi()
{
//    QList<int> keyList = rhkq.keys();
//    for (int index = 0; index < keyList.size(); index++)
//    {
//        int i = keyList.at(index);

//        rhkq.modify(i, [&](Mubiao& mb) { mb.qingchu_weiji_circleppi(); });
//    }

//    dj_point.clear();
    QList<int> keyList = rhkq.keys();
       for (int index = 0; index < keyList.size(); index++)
       {
           int i = keyList.at(index);
           rhkq.modify(i, [&](Mubiao& mb) {
               // 只清除尾迹，但保留最后一个位置点
               if (mb.circleppi_hangji.size() > 10) {
                   QPointF lastPos = mb.circleppi_hangji.last();
                   mb.circleppi_hangji.clear();
                   mb.circleppi_hangji.append(lastPos);
               }
           });
       }

       dj_point.clear();
}

void CirclePPIWidget::PPIGuiLing_clicked()
{
    ppizhongxinx = width() / 2;
    ppizhongxiny = height() / 2;
    m_radius     = 340;
    setNeedRedrawRuler(true);
    QingChu_WeiJi();
}

void CirclePPIWidget::JuBiaoJian_clicked()
{
    huan_ju_wheel--;
    if (huan_ju_wheel < 1)
    {
        huan_ju_wheel = 1;
    }
    huan_ju_cal(huan_ju_wheel);

    setNeedRedrawRuler(true);
}

void CirclePPIWidget::JuBiaoJia_clicked()
{
    huan_ju_wheel++;
    if (huan_ju_wheel > 10)
    {
        huan_ju_wheel = 10;
    }
    huan_ju_cal(huan_ju_wheel);
    setNeedRedrawRuler(true);
}

void CirclePPIWidget::huan_ju_cal(uint8_t num)
{
    switch (num)
    {
        case 1:
            huan_ju = 1000;
            break;
        case 2:
            huan_ju = 2000;
            break;
        case 3:
            huan_ju = 3000;
            break;
        case 4:
            huan_ju = 4000;
            break;
        case 5:
            huan_ju = 5000;
            break;
        case 6:
            huan_ju = 10000;
            break;
        case 7:
            huan_ju = 20000;
            break;
        case 8:
            huan_ju = 30000;
            break;
        case 9:
            huan_ju = 40000;
            break;
        case 10:
            huan_ju = 50000;
            break;
        default:
            break;
    }
    QingChu_WeiJi();
    lbl_jubiao->setText(QString::number((float) (huan_ju * 1.0 / 1000)) + " km");
}

void CirclePPIWidget::GradientArc(int type, QPainter* painter, double start, double stop)
{
    painter->setRenderHint(QPainter::Antialiasing);  // 抗锯齿

    // 扇形参数
    int centerX = ppizhongxinx;
    int centerY = ppizhongxiny;
    // int centerX = 830;
    // int centerY = 449;

    int radius = m_radius;

    // 设置扇形颜色为红色，并调整透明度
    QRadialGradient gradient(centerX, centerY, radius);
    QPen            pen;  // 设置边框颜色

    switch (type)
    {
        case 1:
            gradient.setColorAt(0, QColor(250, 83, 83,
                                          77));  // 红色，透明度127（范围0-255，0完全透明，255完全不透明）
            gradient.setColorAt(1, QColor(250, 83, 83,
                                          77));  // 保持红色且透明度一致，此处假设整个扇形颜色统一，无需渐变
            pen.setColor(QColor(214, 91, 94));
            break;
        case 2:
            gradient.setColorAt(0, QColor(253, 255, 80,
                                          60));  // 红色，透明度127（范围0-255，0完全透明，255完全不透明）
            gradient.setColorAt(1, QColor(253, 255, 80,
                                          60));  // 保持红色且透明度一致，此处假设整个扇形颜色统一，无需渐变
            pen.setColor(QColor(253, 255, 80));
            break;
        case 3:
            gradient.setColorAt(0, QColor(179, 220, 137,
                                          60));  // 红色，透明度127（范围0-255，0完全透明，255完全不透明）
            gradient.setColorAt(1, QColor(179, 220, 137,
                                          60));  // 保持红色且透明度一致，此处假设整个扇形颜色统一，无需渐变
            pen.setColor(QColor(179, 220, 137));
            break;
        case 4:
            gradient.setColorAt(0, QColor(0, 247, 119,
                                          60));  // 红色，透明度127（范围0-255，0完全透明，255完全不透明）
            gradient.setColorAt(1, QColor(0, 247, 119,
                                          60));  // 保持红色且透明度一致，此处假设整个扇形颜色统一，无需渐变
            pen.setColor(QColor(0, 247, 119));
            break;
        case 5:
            gradient.setColorAt(0, QColor(0, 255, 234,
                                          60));  // 红色，透明度127（范围0-255，0完全透明，255完全不透明）
            gradient.setColorAt(1, QColor(0, 255, 234,
                                          60));  // 保持红色且透明度一致，此处假设整个扇形颜色统一，无需渐变
            pen.setColor(QColor(0, 255, 234));
            break;
        default:
            break;
    }

    painter->setBrush(gradient);
    pen.setWidth(1);       // 设置边框宽度
    painter->setPen(pen);  // 应用设置的画笔RGB(214,91,94)
    QRect rect(centerX - radius, centerY - radius, radius * 2, radius * 2);

    if (stop < start)
    {
        stop += 360;
    }

    // 将角度转换为Qt使用的单位
    int qtStartAngle = (90 - start) * 16;
    int qtSpanAngle  = -(stop - start) * 16;

    painter->drawPie(rect, qtStartAngle, qtSpanAngle);
}

/**
 * @brief CirclePPIWidget::drawDashedLine
 * @param painter
 * @param fangwei
 * 绘制虚线
 */
void CirclePPIWidget::drawDashedLine(QPainter* painter, double fangwei)
{
    QPen pen(Qt::red);           // 设置画笔颜色为红色
    pen.setStyle(Qt::DashLine);  // 设置画笔样式为虚线
    painter->setPen(pen);

    // 将角度转换为弧度
    qreal radian = qDegreesToRadians(fangwei);

    int radius = huanjianju * 5 + 5;

    // 计算圆上的点
    qreal x = ppizhongxinx + radius * qCos(radian);
    qreal y = ppizhongxiny + radius * qSin(radian);

    // 绘制虚线
    painter->drawLine(ppizhongxinx, ppizhongxiny, x, y);
}

void CirclePPIWidget::quxiao_pihao_fun(int pihao)
{
    if (auto mb = rhkq.value(pihao))
    {
        if (mb->ShangJiPiHao > 0)
        {
            return;
        }
        if (mb->BenCheGenLeiPiHao > 0)
        {
//            U212_GenZongLeiDaCanShuSheZhi.MuBiaoPiHao              = mb->BenCheGenLeiPiHao;
//            U212_GenZongLeiDaCanShuSheZhi.GenZongLeiDaZhiPaiSheZhi = 3;
//            UdpSend(212, (char*) &U212_GenZongLeiDaCanShuSheZhi, sizeof(U212_GenZongLeiDaCanShuSheZhi));
//            U212_GenZongLeiDaCanShuSheZhi.MuBiaoPiHao              = 0;
//            U212_GenZongLeiDaCanShuSheZhi.GenZongLeiDaZhiPaiSheZhi = 0;
            qDebug() << "send genzongLD data" << endl;
        }
        if (mb->BenCheGenLeiTanCePiHao > 0)
        {
//            U212_GenZongLeiDaCanShuSheZhi.MuBiaoPiHao              = mb->BenCheGenLeiTanCePiHao;
//            U212_GenZongLeiDaCanShuSheZhi.GenZongLeiDaZhiPaiSheZhi = 3;
//            UdpSend(212, (char*) &U212_GenZongLeiDaCanShuSheZhi, sizeof(U212_GenZongLeiDaCanShuSheZhi));
//            U212_GenZongLeiDaCanShuSheZhi.MuBiaoPiHao              = 0;
//            U212_GenZongLeiDaCanShuSheZhi.GenZongLeiDaZhiPaiSheZhi = 0;
            qDebug() << "send genzongLD data" << endl;
        }
        if (mb->BenCheSouLeiPiHao > 0)
        {
//            U206_SouSuoLeiDaCanShuSheZhi.MuBiaoPiHao                    = mb->BenCheSouLeiPiHao;
//            U206_SouSuoLeiDaCanShuSheZhi.ZhongDianGuanZhuYuXiaoPiSheZhi = 3;
//            UdpSend(206, (char*) &U206_SouSuoLeiDaCanShuSheZhi, sizeof(U206_SouSuoLeiDaCanShuSheZhi));
//            U206_SouSuoLeiDaCanShuSheZhi.MuBiaoPiHao                    = 0;
//            U206_SouSuoLeiDaCanShuSheZhi.ZhongDianGuanZhuYuXiaoPiSheZhi = 0;
            qDebug() << "send genzongLD data" << endl;
        }
        QThread::msleep(200);

        U241_RenGongGanYuXinXi.LeiDaXiaoPiPiHao = mb->pihao;
        UdpSend(241, (char*) &U241_RenGongGanYuXinXi, sizeof(U241_RenGongGanYuXinXi));
        U241_RenGongGanYuXinXi.LeiDaXiaoPiPiHao = 0;
        QThread::msleep(200);

        rhkq.modify(pihao, [&](Mubiao& mb) { mb.new_time = 0; });
    }
}

int CirclePPIWidget::get_Current_MuBiao(QPoint clickPos)
{
    uint16_t   x_cha = 20, y_cha = 10;
    QList<int> keyList = rhkq.keys();
    for (int index = 0; index < keyList.size(); index++)
    {
        int                    i  = keyList.at(index);
        QSharedPointer<Mubiao> mb = rhkq.value(i);
        if (mb && !mb->circleppi_hangji.isEmpty())
        {
            x_cha = (uint16_t) (qAbs((clickPos.x() - mb->circleppi_hangji.last().rx())));
            y_cha = (uint16_t) (qAbs((clickPos.y() - mb->circleppi_hangji.last().ry())));
            if ((x_cha <= 5) && (y_cha <= 5))  // 确定目标
            {
                return mb->pihao;
                break;
            }
        }
    }
    return -1;
}

int CirclePPIWidget::calc_zhongxinfw(QPoint clickPos)
{
    short  x_cha, y_cha;
    double data;

    int x = clickPos.x();
    int y = clickPos.y();

    if ((x < (ppizhongxinx - huanjianju * 5)) || (x > (ppizhongxinx + huanjianju * 5)))
    {
        return -1;
    }
    if ((y < (ppizhongxiny - huanjianju * 5)) || (y > (ppizhongxiny + huanjianju * 5)))
    {
        return -1;
    }
    x_cha = (short) (x - ppizhongxinx);
    y_cha = (short) (ppizhongxiny - y);

    double arc;
    /*    if(y_cha < 0)			//2,3象限
        data += PI;
    if(data > (2*PI))
        data -= 2*PI;*/
    if (x_cha == 0)
    {
        if (y_cha >= 0)
        {
            data = 0.0;
        }
        else
        {
            data = PI;
        }
    }
    else if (y_cha == 0)
    {
        if (x_cha >= 0)
        {
            data = 0.5 * PI;
        }
        else
        {
            data = 1.5 * PI;
        }
    }
    else
    {
        arc  = (x_cha * 1.0) / (y_cha * 1.0);
        data = atan(arc);
    }

    zhongxinfw = (short) (data * 3000 / PI);
    if (y_cha < 0)
    {
        if (zhongxinfw != 3000)
        {
            zhongxinfw += 3000;
        }
        if (zhongxinfw > 6000)
        {
            zhongxinfw -= 6000;
        }
    }
    if (zhongxinfw < 0)
    {
        zhongxinfw += 6000;
    }

    return zhongxinfw;
}

void CirclePPIWidget::drawSelectionRect(QPainter& painter)
{
    painter.save();
    painter.setPen(QPen(Qt::red, 2));
    painter.drawRect(QRect(m_startPoint, m_endPoint));
    painter.restore();
}

QPoint CirclePPIWidget::calculateCenterPoint()
{
    int x = (m_startPoint.x() + m_endPoint.x()) / 2;
    int y = (m_startPoint.y() + m_endPoint.y()) / 2;
    return QPoint(x, y);
}

void CirclePPIWidget::paintruler(QPainter* painter)
{
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter->translate(ppizhongxinx, ppizhongxiny);

    QPen   pen;
    QColor linecolor = QColor("#BEBEBE");
    // linecolor.setAlpha(200);
    pen.setColor(linecolor);
    pen.setCapStyle(Qt::RoundCap);
    pen.setWidthF(2);
    painter->setPen(pen);

    // 标环
    {
        qreal huanbiao     = m_radius;
        int   ellipseSpace = huanbiao * 0.2;
        huanjianju         = ellipseSpace;
        for (int i = 0; i < 5; i++)
        {
            int ra = ellipseSpace * (i + 1);
            // 指定圆心，和半径
            painter->drawEllipse(QPoint(0, 0), ra, ra);
            // 最后一圈不绘制距标
            if (i < 4)
            {
                // 新增距标值绘制
                qreal   actualDistance = huan_ju * (i + 1) * 0.001;                                 // 计算实际距离
                QString distanceText   = QString("%1").arg(QString::number((int) actualDistance));  // 生成距离文本
                // // 在四个轴向位置绘制距标值
                painter->drawText(QRectF(ra + 2, 0, 25, 20), Qt::AlignLeft,
                                  distanceText);  // 右轴`
                painter->drawText(QRectF(-ra - 27, 0, 25, 20), Qt::AlignRight,
                                  distanceText);  // 左轴
                painter->drawText(QRectF(3, -ra - 20, 25, 20), Qt::AlignLeft,
                                  distanceText);  // 上轴
                painter->drawText(QRectF(3, ra, 25, 20), Qt::AlignLeft,
                                  distanceText);  // 下轴
            }
        }
    }
    // 刻度
    {
        int   scaleMajor  = 36;
        qreal radius_kedu = m_radius - 6;
        painter->save();
        painter->rotate(0);
        int    subScaleMajor = 10;
        int    midScaleMajor = subScaleMajor / 2;
        int    steps         = (scaleMajor * subScaleMajor);
        double angleStep     = 360.0 / steps;

        for (int i = 0; i <= steps; i++)
        {
            if (i % subScaleMajor == 0)
            {
                painter->drawLine(0, radius_kedu - 15, 0, radius_kedu);

                if (i % (subScaleMajor * 9) == 0)
                {
                    //                    pen.setWidthF(2.0);
                    //                    QColor linecolor = QColor("#BEBEBE");
                    //                    linecolor.setAlpha(100);
                    //                    pen.setColor(linecolor);
                    //                    painter->setPen(pen);
                    painter->drawLine(0, 0, 0, radius_kedu);
                }
            }
            else if (i % midScaleMajor == 0)
            {
                pen.setWidthF(2);
                painter->setPen(pen);
                painter->drawLine(0, radius_kedu - 10, 0, radius_kedu);
            }
            else
            {
                pen.setWidthF(1);
                painter->setPen(pen);
                painter->drawLine(0, radius_kedu - 5, 0, radius_kedu);
            }
            painter->rotate(angleStep);
        }
        painter->restore();
    }

    // 表盘
    {
        QFont font = painter->font();
        font.setPointSize(9);
        font.setBold(true);
        painter->setFont(font);
        QPen   pen;
        QColor linecolor = QColor("#BEBEBE");
        // linecolor.setAlpha(200);
        pen.setColor(linecolor);
        painter->setPen(pen);
        qreal        radius     = m_radius;  // 缩小绘制半径
        int          scaleMajor = 12;
        QFontMetrics fm(painter->font());

        for (int i = 0; i < scaleMajor; i++)
        {
            // 修改角度基准：0°指向正上方（12点钟方向）
            qreal angle   = 90.0 - i * 30.0;  // 从正上方开始顺时针计算
            qreal radians = qDegreesToRadians(angle);

            // 调整坐标计算
            qreal x = radius * cos(radians);
            qreal y = -(radius + 5) * sin(radians);

            // 优化区域判断逻辑（基于新的角度基准）
            Qt::Alignment align = Qt::AlignCenter;

            if (i == 1 || i == 2)
            {  // 右上区域
                x += 13;
                align = Qt::AlignLeft | Qt::AlignVCenter;
            }
            else if (i == 3 || i == 4 || i == 5)
            {  // 右下区域
                x += 13;
                align = Qt::AlignRight | Qt::AlignVCenter;
            }
            else if (i == 7 || i == 8)
            {  // 左下区域
                x -= 13;
                align = Qt::AlignHCenter | Qt::AlignBottom;
            }
            else if (i == 9 || i == 10 || i == 11)
            {  // 左上区域
                x -= 13;
                align = Qt::AlignHCenter | Qt::AlignTop;
            }
            else
            {
                ;  // 0和180度不做偏移
            }

            QString text      = QString::number((i * 30) % 360);
            int     textWidth = fm.horizontalAdvance(text);
            QRect   textRect(x - textWidth / 2 - 2, y - 10, textWidth + 4, 20);
            painter->drawText(textRect, align, text);
        }
    }
    painter->restore();
}

void CirclePPIWidget::generateTestTargets()
{
    static bool initialized = false;
    if (!initialized)
    {
        for (int i = 1; i <= 10; ++i)
                {
                    try {
                        QSharedPointer<Mubiao> target = QSharedPointer<Mubiao>::create();
                        if (!target) {
                            qDebug() << "创建目标" << i << "失败";
                            continue;
                        }

                        target->pihao = i;
                        target->fangwei = i * 300; // 均匀分布在圆周上
                        target->gaodi = 0;
                        target->juli = 1000 + i * 200; // 不同距离

                        // 计算初始位置
                        QPointF pos = get_xyz(target->fangwei, target->gaodi, target->juli);
                        if (!target->circleppi_hangji.isEmpty()) {
                            target->circleppi_hangji.clear();
                        }
                        target->circleppi_hangji.append(pos);

                        bool insertSuccess = false;
                        for (int attempt = 0; attempt < 3; ++attempt) {
                            if (rhkq.contains(i)) {
                                rhkq.remove(i);
                            }
                            rhkq.insert(i, target);
                            if (rhkq.contains(i)) {
                                insertSuccess = true;
                                break;
                            }
                            QThread::msleep(10);
                        }

                        if (!insertSuccess) {
                            qDebug() << "插入目标" << i << "到rhkq失败";
                        }
                    }
                    catch (const std::exception& e) {
                        qDebug() << "生成目标" << i << "时发生异常:" << e.what();
                    }
                    catch ( ...) {
                        qDebug() << "生成目标" << i << "时发生未知异常";
                    }
                }
                generateTestDianJi();
                qDebug() << "测试目标生成完成，共生成" << rhkq.size() << "个目标";
                initialized = true;
    }else
    {
        // 每次更新目标位置，模拟目标移动
        QList<int> keyList = rhkq.keys();
        for (int index = 0; index < keyList.size(); index++)
        {
            int i = keyList.at(index);
            rhkq.modify(i, [&](Mubiao& mb) {
                // 稍微移动目标位置
                mb.fangwei += 1.0; // 每次旋转1度
                if (mb.fangwei > 6000) mb.fangwei = 0;

                // 更新位置
                QPointF newPos = get_xyz(mb.fangwei, mb.gaodi, mb.juli);

                // 保持航迹长度不超过10个点
                if (mb.circleppi_hangji.size() >= 10) {
                    mb.circleppi_hangji.clear();
                }
                mb.circleppi_hangji.append(newPos);
            });
        }
        generateTestDianJi();
    }
}

// 生成测试点迹数据
void CirclePPIWidget::generateTestDianJi()
{
    // 清空现有点迹
    dj_point.clear();

    // 生成随机点迹
    for (int i = 0; i < 50; ++i) {
        QVector<QPointF> dianjiGroup;

        // 随机方位角 (0-6000密位)
        float randomFangwei = qrand() % 6000;
        // 随机距离 (1-5km)
        uint32_t randomJuli = 1000 + (qrand() % 4000);

        // 计算点迹位置
        QPointF pos = get_xyz(randomFangwei, 0, randomJuli);
        dianjiGroup.append(pos);

        // 添加一些随机偏移的点，模拟点迹散布
        for (int j = 0; j < 3; ++j) {
            QPointF offsetPos = pos + QPointF((qrand() % 10) - 5, (qrand() % 10) - 5);
            dianjiGroup.append(offsetPos);
        }

        dj_point.append(dianjiGroup);
    }

    qDebug() << "生成测试点迹数据，共" << dj_point.size() << "组点迹";
}

// 更新点迹数据
void CirclePPIWidget::updateTestDianJi()
{
    static int updateCount = 0;
    updateCount++;

    // 每5次更新重新生成一次点迹，模拟点迹更新
    if (updateCount % 5 == 0) {
        // 保留部分历史点迹，添加新点迹
        QVector<QVector<QPointF>> currentDianji = dj_point.get_data();

        // 如果点迹太多，移除一些旧的
        if (currentDianji.size() > 30) {
            currentDianji.remove(0, currentDianji.size() - 30);
        }

        // 添加新点迹
        for (int i = 0; i < 10; ++i) {
            QVector<QPointF> dianjiGroup;

            // 随机方位角 (0-6000密位)
            float randomFangwei = qrand() % 6000;
            // 随机距离 (1-5km)
            uint32_t randomJuli = 1000 + (qrand() % 4000);

            // 计算点迹位置
            QPointF pos = get_xyz(randomFangwei, 0, randomJuli);
            dianjiGroup.append(pos);

            // 添加一些随机偏移的点
            for (int j = 0; j < 2; ++j) {
                QPointF offsetPos = pos + QPointF((qrand() % 8) - 4, (qrand() % 8) - 4);
                dianjiGroup.append(offsetPos);
            }

            currentDianji.append(dianjiGroup);
        }

        // 更新点迹数据
        dj_point.clear();
        for (const auto& group : currentDianji) {
            dj_point.append(group);
        }

        qDebug() << "更新点迹数据，当前点迹组数:" << dj_point.size();
    }
}
