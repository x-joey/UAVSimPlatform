#include "PPIGraphicsItem.h"
#include "MubiaoAdapter.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QFontMetricsF>
#include <QCursor>
#include <QtMath>
#include <QDebug>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PPIGraphicsItem::PPIGraphicsItem(QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , m_dj_point(100)
{
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);

    // 初始化定时器
    m_updateTimer = new QTimer();
    connect(m_updateTimer, &QTimer::timeout, this, &PPIGraphicsItem::onUpdateTimer);
    m_updateTimer->start(200); // 200ms更新一次（5Hz）

    // 初始化标尺缓存（根据半径动态调整）
    updateRulerBufferSize();
}

PPIGraphicsItem::~PPIGraphicsItem()
{
    if (m_updateTimer) {
        m_updateTimer->stop();
        delete m_updateTimer;
    }
}

QRectF PPIGraphicsItem::boundingRect() const
{
    // 返回PPI的包围矩形
    double size = m_radius * 2 + 100; // 留出边距
    return QRectF(-size/2, -size/2, size, size);
}

void PPIGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 设置透明度
    painter->setOpacity(m_opacity);

    // 填充黑色背景（如果启用）
    if (m_drawBackground) {
        painter->fillRect(boundingRect(), Qt::black);
    }

    // 绘制标尺（如果需要重绘）
    if (m_needRedrawRuler)
    {
        m_rulerBuffer.fill(Qt::transparent);
        QPainter rulerPainter(&m_rulerBuffer);
        rulerPainter.setRenderHint(QPainter::Antialiasing, true);
        rulerPainter.translate(m_rulerBuffer.width() / 2, m_rulerBuffer.height() / 2);
        paintRuler(&rulerPainter);
        m_needRedrawRuler = false;
    }

    // 绘制缓存的标尺（将缓存中心对齐到原点）
    painter->save();
    QPointF topLeft(-m_rulerBuffer.width() / 2.0, -m_rulerBuffer.height() / 2.0);
    painter->drawPixmap(topLeft, m_rulerBuffer);
    painter->restore();

    // 绘制扇形区域
    paintSectors(painter);

    // 绘制火力线
    paintFireLine(painter);

    // 绘制目标
    paintTargets(painter);

    // 绘制点迹
    if (m_show_dianji)
    {
        paintDianji(painter);
    }
}

void PPIGraphicsItem::paintRuler(QPainter* painter)
{
    painter->save();

    QPen pen;
    QColor linecolor = QColor("#BEBEBE");
    pen.setColor(linecolor);
    pen.setCapStyle(Qt::RoundCap);
    pen.setWidthF(2);
    painter->setPen(pen);

    // 1. 绘制距离环
    double ellipseSpace = m_radius * 0.2;
    m_huanjianju = static_cast<int>(ellipseSpace);

    for (int i = 0; i < 5; i++)
    {
        int ra = ellipseSpace * (i + 1);
        painter->drawEllipse(QPoint(0, 0), ra, ra);

        // 绘制距离标注（前4个圆环）
        if (i < 4)
        {
            double actualDistance = m_huan_ju * (i + 1) * 0.001; // 转换为km
            QString distanceText = QString::number(static_cast<int>(actualDistance));

            // 在四个方向绘制距离值
            painter->drawText(QRectF(ra + 2, -10, 25, 20), Qt::AlignLeft, distanceText);    // 右
            painter->drawText(QRectF(-ra - 27, -10, 25, 20), Qt::AlignRight, distanceText); // 左
            painter->drawText(QRectF(-12, -ra - 20, 25, 20), Qt::AlignCenter, distanceText);// 上
            painter->drawText(QRectF(-12, ra, 25, 20), Qt::AlignCenter, distanceText);      // 下
        }
    }

    // 2. 绘制刻度线
    double radius_kedu = m_radius - 6;
    int scaleMajor = 36;
    int subScaleMajor = 10;
    int steps = scaleMajor * subScaleMajor;
    double angleStep = 360.0 / steps;

    painter->save();
    for (int i = 0; i <= steps; i++)
    {
        if (i % subScaleMajor == 0)
        {
            // 主刻度线
            painter->drawLine(0, -radius_kedu + 15, 0, -radius_kedu);

            if (i % (subScaleMajor * 9) == 0)
            {
                // 每90度绘制径向线
                painter->drawLine(0, 0, 0, -radius_kedu);
            }
        }
        else if (i % (subScaleMajor / 2) == 0)
        {
            // 中刻度线
            painter->drawLine(0, -radius_kedu + 10, 0, -radius_kedu);
        }
        else
        {
            // 小刻度线
            painter->drawLine(0, -radius_kedu + 5, 0, -radius_kedu);
        }
        painter->rotate(angleStep);
    }
    painter->restore();

    // 3. 绘制方位盘（0-360度）
    QFont font = painter->font();
    font.setPointSize(9);
    font.setBold(true);
    painter->setFont(font);
    QFontMetricsF fm(font);

    for (int i = 0; i < 12; i++)
    {
        // 从正北方向开始（0度在上方）
        double angle = 90.0 - i * 30.0;
        double radians = qDegreesToRadians(angle);

        double x = (m_radius + 5) * cos(radians);
        double y = -(m_radius + 5) * sin(radians);

        QString text = QString::number((i * 30) % 360);
        int textWidth = fm.horizontalAdvance(text);

        // 根据位置调整文本对齐
        Qt::Alignment align = Qt::AlignCenter;
        if (i == 1 || i == 2) {
            x += 13;
            align = Qt::AlignLeft | Qt::AlignVCenter;
        } else if (i == 4 || i == 5) {
            x += 13;
            align = Qt::AlignRight | Qt::AlignVCenter;
        } else if (i == 7 || i == 8) {
            x -= 13;
            align = Qt::AlignHCenter | Qt::AlignBottom;
        } else if (i == 10 || i == 11) {
            x -= 13;
            align = Qt::AlignHCenter | Qt::AlignTop;
        }

        QRect textRect(x - textWidth / 2 - 2, y - 10, textWidth + 4, 20);
        painter->drawText(textRect, align, text);
    }

    painter->restore();
}

void PPIGraphicsItem::paintFireLine(QPainter* painter)
{
    painter->save();

    // 旋转到火力线方向
    painter->rotate(m_HuoLiXian_MaPan);

    // 绘制火力线
    QPen linePen(QColor(0, 255, 234, 170), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(linePen);
    painter->setBrush(QColor(0, 255, 234));

    // 绘制线条
    painter->drawLine(QPointF(0, 0), QPointF(0, -m_radius));

    // 绘制箭头
    const int arrowHeadSize = 8;
    QPointF arrowEnd(0, -m_radius);
    QPolygonF arrowHead;
    arrowHead << arrowEnd
              << QPointF(-arrowHeadSize, arrowEnd.y() + arrowHeadSize)
              << QPointF(arrowHeadSize, arrowEnd.y() + arrowHeadSize);
    painter->drawPolygon(arrowHead);

    painter->restore();
}

void PPIGraphicsItem::paintTargets(QPainter* painter)
{
    painter->save();

    QList<int> keyList = m_rhkq.keys();
    for (int pihao : keyList)
    {
        QSharedPointer<Mubiao> mb = m_rhkq.value(pihao);
        if (!mb || mb->daji_flag) continue; // 跳过已打击目标

        // 获取目标航迹
        QVector<QPointF> trajectory = mb->circleppi_hangji.get_data();
        if (trajectory.isEmpty()) continue;

        QPointF pos = trajectory.last();

        // 绘制目标符号
        mb->draw_mubiao(painter, pos);

        // 绘制航迹点
        QPen trackPen(QColor(100, 100, 100), 2);
        painter->setPen(trackPen);
        for (const QPointF& point : trajectory)
        {
            painter->drawPoint(point);
        }

        // 如果是重点目标，绘制连线到航迹
        if (mb->zhongdian && trajectory.size() > 1)
        {
            QPen linePen(Qt::yellow, 1, Qt::DashLine);
            painter->setPen(linePen);
            painter->drawPolyline(QPolygonF::fromList(trajectory.toList()));
        }
    }

    painter->restore();
}

void PPIGraphicsItem::paintDianji(QPainter* painter)
{
    painter->save();

    QVector<QVector<QPointF>> dianjiData = m_dj_point.get_data();

    painter->setRenderHint(QPainter::Antialiasing, true);

    for (int i = 0; i < dianjiData.size(); i++)
    {
        const QVector<QPointF>& currentDjPoints = dianjiData[i];

        // 使用不同颜色区分不同组
        int colorValue = (i * 50) % 255;
        QColor pointColor = QColor::fromRgb(
            (colorValue + 100) % 255,
            (colorValue + 50) % 255,
            colorValue
        );

        QPen pointPen(pointColor, 3);
        pointPen.setCapStyle(Qt::RoundCap);
        painter->setPen(pointPen);

        // 绘制点迹
        for (const QPointF& point : currentDjPoints)
        {
            painter->drawPoint(point);
        }
    }

    painter->restore();
}

void PPIGraphicsItem::paintSectors(QPainter* painter)
{
    painter->save();

    // 绘制禁射区域（红色）
    for (const JinSheQuYu& item : m_JSQY_list)
    {
        drawGradientArc(painter, 1, item.ZuoBianJie, item.YouBianJie);
    }

    for (const JinSheQuYu& item : m_FX_JS_list)
    {
        drawGradientArc(painter, 1, item.ZuoBianJie, item.YouBianJie);
    }

    for (const JinSheQuYu& item : m_QY_JS_list)
    {
        drawGradientArc(painter, 1, item.ZuoBianJie, item.YouBianJie);
    }

    // 绘制责任扇区（青色）
    if (m_Is_ZeRenShanQu_Use)
    {
        double start = m_BenDiZeRenShanQu.sq_start * 360.0 / 6000.0;
        double end = m_BenDiZeRenShanQu.sq_end * 360.0 / 6000.0;
        drawGradientArc(painter, 4, start, end);
    }

    painter->restore();
}

void PPIGraphicsItem::drawGradientArc(QPainter* painter, int type, double start, double stop)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    int radius = m_radius;

    // 设置渐变和颜色
    QRadialGradient gradient(0, 0, radius);
    QPen pen;

    switch (type)
    {
    case 1: // 禁射区域（红色）
        gradient.setColorAt(0, QColor(250, 83, 83, 77));
        gradient.setColorAt(1, QColor(250, 83, 83, 77));
        pen.setColor(QColor(214, 91, 94));
        break;
    case 2: // 干扰屏蔽区（黄色）
        gradient.setColorAt(0, QColor(253, 255, 80, 60));
        gradient.setColorAt(1, QColor(253, 255, 80, 60));
        pen.setColor(QColor(253, 255, 80));
        break;
    case 3: // 静默扇区（绿色）
        gradient.setColorAt(0, QColor(179, 220, 137, 60));
        gradient.setColorAt(1, QColor(179, 220, 137, 60));
        pen.setColor(QColor(179, 220, 137));
        break;
    case 4: // 责任扇区（青色）
        gradient.setColorAt(0, QColor(0, 247, 119, 60));
        gradient.setColorAt(1, QColor(0, 247, 119, 60));
        pen.setColor(QColor(0, 247, 119));
        break;
    default:
        painter->restore();
        return;
    }

    painter->setBrush(gradient);
    pen.setWidth(1);
    painter->setPen(pen);

    QRect rect(-radius, -radius, radius * 2, radius * 2);

    // 处理跨越360度的情况
    if (stop < start)
    {
        stop += 360;
    }

    // 转换为Qt使用的单位（1/16度）
    int qtStartAngle = (90 - start) * 16;
    int qtSpanAngle = -(stop - start) * 16;

    painter->drawPie(rect, qtStartAngle, qtSpanAngle);

    painter->restore();
}

QPointF PPIGraphicsItem::polarToCartesian(float fangwei, uint32_t juli)
{
    return MubiaoAdapter::polarToCartesian(fangwei, juli, 0, 0, m_radius, m_huan_ju);
}

int PPIGraphicsItem::getCurrentTarget(const QPointF& pos)
{
    QList<int> keyList = m_rhkq.keys();
    for (int pihao : keyList)
    {
        QSharedPointer<Mubiao> mb = m_rhkq.value(pihao);
        if (!mb) continue;

        QVector<QPointF> trajectory = mb->circleppi_hangji.get_data();
        if (trajectory.isEmpty()) continue;

        QPointF targetPos = trajectory.last();
        double distance = QLineF(pos, targetPos).length();

        if (distance <= 10.0) // 10像素容差
        {
            return mb->pihao;
        }
    }
    return -1;
}

void PPIGraphicsItem::setRadius(double radius)
{
    if (radius < 100) radius = 100;
    if (radius > 3000) radius = 3000;

    m_radius = radius;
    updateRulerBufferSize();
    m_needRedrawRuler = true;
    prepareGeometryChange();
    update();
}

void PPIGraphicsItem::setHuanJu(uint32_t huanJu)
{
    m_huan_ju = huanJu;
    m_needRedrawRuler = true;
    update();
}

void PPIGraphicsItem::setPPIOpacity(double opacity)
{
    if (opacity < 0.0) opacity = 0.0;
    if (opacity > 1.0) opacity = 1.0;

    m_opacity = opacity;
    update();
}

void PPIGraphicsItem::setDraggable(bool draggable)
{
    m_draggable = draggable;
    setFlag(QGraphicsItem::ItemIsMovable, draggable);

    // 如果启用拖动，需要修改鼠标交互模式
    if (draggable) {
        setCursor(QCursor(Qt::OpenHandCursor));
    } else {
        setCursor(QCursor(Qt::ArrowCursor));
    }
}

void PPIGraphicsItem::addJinSheQuYu(double left, double right)
{
    JinSheQuYu qu;
    qu.ZuoBianJie = left;
    qu.YouBianJie = right;
    m_JSQY_list.append(qu);
    update();
}

void PPIGraphicsItem::clearJinSheQuYu()
{
    m_JSQY_list.clear();
    m_FX_JS_list.clear();
    m_QY_JS_list.clear();
    update();
}

void PPIGraphicsItem::setNeedRedrawRuler(bool need)
{
    m_needRedrawRuler = need;
    if (need)
    {
        update();
    }
}

void PPIGraphicsItem::onUpdateTimer()
{
    m_time_count++;

    // 更新火力线旋转（可选）
    // m_HuoLiXian_MaPan += 0.5;
    // if (m_HuoLiXian_MaPan >= 360) m_HuoLiXian_MaPan = 0;

    update();
}

// 事件处理
void PPIGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    m_lastClickPos = event->pos();

    if (m_draggable) {
        // 拖动模式：改变鼠标样式并调用基类处理
        setCursor(QCursor(Qt::ClosedHandCursor));
        QGraphicsItem::mousePressEvent(event);
    } else {
        // 不拖动：不调用基类，避免触发移动
        event->accept();
    }
}

void PPIGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    QPointF pos = event->pos();
    int targetId = getCurrentTarget(pos);

    if (targetId > 0)
    {
        emit targetDoubleClicked(targetId);
    }

    QGraphicsItem::mouseDoubleClickEvent(event);
}

void PPIGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QPointF pos = event->pos();

    if (m_draggable) {
        // 拖动模式：恢复鼠标样式
        setCursor(QCursor(Qt::OpenHandCursor));
        QGraphicsItem::mouseReleaseEvent(event);
    } else {
        // 非拖动模式：检测是否点击了目标
        if (QLineF(pos, m_lastClickPos).length() < 5.0) // 判断是点击而非拖拽
        {
            int targetId = getCurrentTarget(pos);
            if (targetId > 0)
            {
                emit targetClicked(targetId);
            }
        }
        event->accept();
    }
}

void PPIGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_draggable) {
        // 拖动模式：调用基类处理移动
        QGraphicsItem::mouseMoveEvent(event);
    } else {
        // 非拖动模式：不处理移动
        event->accept();
    }
}

void PPIGraphicsItem::wheelEvent(QGraphicsSceneWheelEvent* event)
{
    // 滚轮缩放PPI半径
    QPoint numDegrees = event->delta() > 0 ? QPoint(0, 120) : QPoint(0, -120);

    if (numDegrees.y() > 0)
    {
        setRadius(m_radius + 20);
    }
    else
    {
        setRadius(m_radius - 20);
    }

    event->accept();
}

void PPIGraphicsItem::updateRulerBufferSize()
{
    // 根据半径动态调整缓存大小
    int bufferSize = static_cast<int>(m_radius * 2 + 200); // 留出边距
    if (bufferSize < 800) bufferSize = 800;
    if (bufferSize > 6400) bufferSize = 6400;

    m_rulerBuffer = QPixmap(bufferSize, bufferSize);
    m_rulerBuffer.fill(Qt::transparent);
    m_needRedrawRuler = true;
}
