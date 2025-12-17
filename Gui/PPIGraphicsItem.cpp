#include "PPIGraphicsItem.h"
#include "MubiaoAdapter.h"
#include <QCursor>
#include <QDebug>
#include <QFontMetricsF>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QtMath>

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

PPIGraphicsItem::PPIGraphicsItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_dj_point(100)
{
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);

    // 初始化定时器
    m_updateTimer = new QTimer();
    connect(m_updateTimer, &QTimer::timeout, this, &PPIGraphicsItem::onUpdateTimer);
    m_updateTimer->start(200);   // 200ms更新一次（5Hz）

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
    double size = m_radius * 2 + 100;   // 留出边距
    return QRectF(-size / 2, -size / 2, size, size);
}

void PPIGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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
    if (m_needRedrawRuler) {
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
    if (m_show_dianji) {
        paintDianji(painter);
    }
}
/**
 * @brief PPIGraphicsItem::paintRuler 绘制距离环
 * @param painter
 */
void PPIGraphicsItem::paintRuler(QPainter *painter)
{
    painter->save();

    QPen   pen;
    QColor linecolor = QColor("#BEBEBE");
    pen.setColor(linecolor);
    pen.setCapStyle(Qt::RoundCap);
    pen.setWidthF(2);
    painter->setPen(pen);

    // 1. 绘制距离环
    double ellipseSpace = m_radius * 0.2;
    m_huanjianju        = static_cast<int>(ellipseSpace);

    for (int i = 0; i < 5; i++) {
        int ra = ellipseSpace * (i + 1);
        painter->drawEllipse(QPoint(0, 0), ra, ra);

        // 绘制距离标注（前4个圆环）
        if (i < 4) {
            double  actualDistance = m_huan_ju * (i + 1) * 0.001;   // 转换为km
            QString distanceText   = QString::number(static_cast<int>(actualDistance));

            // 在四个方向绘制距离值
            painter->drawText(QRectF(ra + 2, -10, 25, 20), Qt::AlignLeft, distanceText);       // 右
            painter->drawText(QRectF(-ra - 27, -10, 25, 20), Qt::AlignRight, distanceText);    // 左
            painter->drawText(QRectF(-12, -ra - 20, 25, 20), Qt::AlignCenter, distanceText);   // 上
            painter->drawText(QRectF(-12, ra, 25, 20), Qt::AlignCenter, distanceText);         // 下
        }
    }

    // 2. 绘制刻度线
    double radius_kedu   = m_radius - 6;
    int    scaleMajor    = 36;
    int    subScaleMajor = 10;
    int    steps         = scaleMajor * subScaleMajor;
    double angleStep     = 360.0 / steps;

    painter->save();
    for (int i = 0; i <= steps; i++) {
        if (i % subScaleMajor == 0) {
            // 主刻度线
            painter->drawLine(0, -radius_kedu + 15, 0, -radius_kedu);

            if (i % (subScaleMajor * 9) == 0) {
                // 每90度绘制径向线
                painter->drawLine(0, 0, 0, -radius_kedu);
            }
        }
        else if (i % (subScaleMajor / 2) == 0) {
            // 中刻度线
            painter->drawLine(0, -radius_kedu + 10, 0, -radius_kedu);
        }
        else {
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

    for (int i = 0; i < 12; i++) {
        // 从正北方向开始（0度在上方）
        double angle   = 90.0 - i * 30.0;
        double radians = qDegreesToRadians(angle);

        double x = (m_radius + 5) * cos(radians);
        double y = -(m_radius + 5) * sin(radians);

        QString text      = QString::number((i * 30) % 360);
        int     textWidth = fm.horizontalAdvance(text);

        // 根据位置调整文本对齐
        Qt::Alignment align = Qt::AlignCenter;
        if (i == 1 || i == 2) {
            x += 13;
            align = Qt::AlignLeft | Qt::AlignVCenter;
        }
        else if (i == 4 || i == 5) {
            x += 13;
            align = Qt::AlignRight | Qt::AlignVCenter;
        }
        else if (i == 7 || i == 8) {
            x -= 13;
            align = Qt::AlignHCenter | Qt::AlignBottom;
        }
        else if (i == 10 || i == 11) {
            x -= 13;
            align = Qt::AlignHCenter | Qt::AlignTop;
        }

        QRect textRect(x - textWidth / 2 - 2, y - 10, textWidth + 4, 20);
        painter->drawText(textRect, align, text);
    }

    painter->restore();
}

/**
 * @brief PPIGraphicsItem::paintFireLine 绘制火力线
 * @param painter
 */
void PPIGraphicsItem::paintFireLine(QPainter *painter)
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
    QPointF   arrowEnd(0, -m_radius);
    QPolygonF arrowHead;
    arrowHead << arrowEnd << QPointF(-arrowHeadSize, arrowEnd.y() + arrowHeadSize) << QPointF(arrowHeadSize, arrowEnd.y() + arrowHeadSize);
    painter->drawPolygon(arrowHead);

    painter->restore();
}

/**
 * @brief PPIGraphicsItem::paintTargets绘制目标
 * @param painter
 */
void PPIGraphicsItem::paintTargets(QPainter *painter)
{
    painter->save();

    QList<int> keyList = m_rhkq.keys();
    for (int pihao : keyList) {
        QSharedPointer<Mubiao> mb = m_rhkq.value(pihao);
        if (!mb || mb->daji_flag)
            continue;   // 跳过已打击目标

        // 获取目标航迹（物理坐标，米），按当前量程/半径实时投影到显示坐标
        QVector<QPointF> trajectoryMeters = mb->circleppi_hangji.get_data();
        if (trajectoryMeters.isEmpty())
            continue;

        QVector<QPointF> trajectoryDisplay;
        trajectoryDisplay.reserve(trajectoryMeters.size());

        for (const QPointF &meterPt : trajectoryMeters) {
            float    fw;
            uint32_t jl;
            MubiaoAdapter::cartesianToPolar(meterPt.x(), meterPt.y(), fw, jl);
            trajectoryDisplay.append(MubiaoAdapter::polarToCartesian(fw, jl, 0.0, 0.0, m_radius, m_huan_ju));
        }

        QPointF pos = trajectoryDisplay.last();

        // 绘制目标符号
        mb->draw_mubiao(painter, pos);

        // 绘制目标标牌（ID标签）
        QPointF labelPos = pos + mb->labelOffset;

        // 绘制连接线（虚线）
        QPen labelLinePen(QColor(150, 150, 150, 200), 1, Qt::DashLine);
        painter->setPen(labelLinePen);
        painter->drawLine(pos, labelPos);

        // 绘制标牌背景
        QString labelText = QString::number(mb->pihao);
        QFont   labelFont = painter->font();
        labelFont.setPointSize(9);
        labelFont.setBold(true);
        painter->setFont(labelFont);

        QFontMetricsF fm(labelFont);
        QRectF        textRect = fm.boundingRect(labelText);
        QRectF        bgRect   = textRect.adjusted(-4, -2, 4, 2);
        bgRect.moveCenter(labelPos);

        // 根据状态设置标牌背景色
        QColor bgColor;
        if (mb->daoyin_flag) {
            bgColor = QColor(255, 200, 200, 230);   // 淡红色背景
        }
        else if (mb->zhongdian) {
            bgColor = QColor(255, 255, 200, 230);   // 淡黄色背景
        }
        else {
            bgColor = QColor(200, 255, 200, 230);   // 淡绿色背景
        }

        painter->setBrush(QBrush(bgColor));
        painter->setPen(QPen(QColor(100, 100, 100), 1));
        painter->drawRoundedRect(bgRect, 3, 3);

        // 绘制标牌文本
        painter->setPen(Qt::black);
        painter->drawText(bgRect, Qt::AlignCenter, labelText);

        // 绘制航迹点（比目标小）
        QPen trackPen(QColor(100, 100, 100, 150), 1);   // 半透明灰色
        painter->setPen(trackPen);
        painter->setBrush(QBrush(QColor(100, 100, 100, 100)));   // 半透明填充

        for (const QPointF &point : trajectoryDisplay) {
            painter->drawEllipse(point, 1, 1);   // 半径1像素，比目标小
        }

        // 如果是重点目标或导引目标，绘制连线到航迹
        if (mb->zhongdian || mb->daoyin_flag) {
            QPen linePen;
            if (mb->daoyin_flag) {
                // 导引中：红色虚线
                linePen = QPen(Qt::red, 1.5, Qt::DashLine);
            }
            else {
                // 重点关注：黄色虚线
                linePen = QPen(Qt::yellow, 1, Qt::DashLine);
            }
            painter->setPen(linePen);

            if (trajectoryDisplay.size() > 1) {
                painter->drawPolyline(QPolygonF::fromList(trajectoryDisplay.toList()));
            }
        }
    }

    painter->restore();
}

void PPIGraphicsItem::paintDianji(QPainter *painter)
{
    painter->save();

    QVector<QVector<QPointF>> dianjiData = m_dj_point.get_data();

    painter->setRenderHint(QPainter::Antialiasing, true);

    for (int i = 0; i < dianjiData.size(); i++) {
        const QVector<QPointF> &currentDjPoints = dianjiData[i];

        // 使用不同颜色区分不同组
        int    colorValue = (i * 50) % 255;
        QColor pointColor = QColor::fromRgb((colorValue + 100) % 255, (colorValue + 50) % 255, colorValue);

        QPen pointPen(pointColor, 1);
        pointPen.setCapStyle(Qt::RoundCap);
        painter->setPen(pointPen);

        // 绘制点迹
        for (const QPointF &point : currentDjPoints) {
            painter->drawPoint(point);
        }
    }

    painter->restore();
}

/**
 * @brief PPIGraphicsItem::paintSectors 绘制区域
 * @param painter
 */
void PPIGraphicsItem::paintSectors(QPainter *painter)
{
    painter->save();

    // 绘制禁射区域（红色）
    for (const JinSheQuYu &item : m_JSQY_list) {
        drawGradientArc(painter, 1, item.ZuoBianJie, item.YouBianJie);
    }

    for (const JinSheQuYu &item : m_FX_JS_list) {
        drawGradientArc(painter, 1, item.ZuoBianJie, item.YouBianJie);
    }

    for (const JinSheQuYu &item : m_QY_JS_list) {
        drawGradientArc(painter, 1, item.ZuoBianJie, item.YouBianJie);
    }

    // 绘制责任扇区（青色）
    if (m_Is_ZeRenShanQu_Use) {
        double start = m_BenDiZeRenShanQu.sq_start * 360.0 / 6000.0;
        double end   = m_BenDiZeRenShanQu.sq_end * 360.0 / 6000.0;
        drawGradientArc(painter, 4, start, end);
    }

    painter->restore();
}

void PPIGraphicsItem::drawGradientArc(QPainter *painter, int type, double start, double stop)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    int radius = m_radius;

    // 设置渐变和颜色
    QRadialGradient gradient(0, 0, radius);
    QPen            pen;

    switch (type) {
    case 1:   // 禁射区域（红色）
        gradient.setColorAt(0, QColor(250, 83, 83, 77));
        gradient.setColorAt(1, QColor(250, 83, 83, 77));
        pen.setColor(QColor(214, 91, 94));
        break;
    case 2:   // 干扰屏蔽区（黄色）
        gradient.setColorAt(0, QColor(253, 255, 80, 60));
        gradient.setColorAt(1, QColor(253, 255, 80, 60));
        pen.setColor(QColor(253, 255, 80));
        break;
    case 3:   // 静默扇区（绿色）
        gradient.setColorAt(0, QColor(179, 220, 137, 60));
        gradient.setColorAt(1, QColor(179, 220, 137, 60));
        pen.setColor(QColor(179, 220, 137));
        break;
    case 4:   // 责任扇区（青色）
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
    if (stop < start) {
        stop += 360;
    }

    // 转换为Qt使用的单位（1/16度）
    int qtStartAngle = (90 - start) * 16;
    int qtSpanAngle  = -(stop - start) * 16;

    painter->drawPie(rect, qtStartAngle, qtSpanAngle);

    painter->restore();
}

QPointF PPIGraphicsItem::polarToCartesian(float fangwei, uint32_t juli)
{
    return MubiaoAdapter::polarToCartesian(fangwei, juli, 0, 0, m_radius, m_huan_ju);
}

int PPIGraphicsItem::getCurrentTarget(const QPointF &pos)
{
    QList<int> keyList = m_rhkq.keys();
    for (int pihao : keyList) {
        QSharedPointer<Mubiao> mb = m_rhkq.value(pihao);
        if (!mb || mb->daji_flag)
            continue;

        // 获取目标航迹（物理坐标，米）
        QVector<QPointF> trajectoryMeters = mb->circleppi_hangji.get_data();
        if (trajectoryMeters.isEmpty())
            continue;

        // 将最后一个物理坐标点转换为显示坐标（像素）
        QPointF  meterPt = trajectoryMeters.last();
        float    fw;
        uint32_t jl;
        MubiaoAdapter::cartesianToPolar(meterPt.x(), meterPt.y(), fw, jl);
        QPointF targetDisplayPos = MubiaoAdapter::polarToCartesian(fw, jl, 0.0, 0.0, m_radius, m_huan_ju);

        // 计算鼠标点击位置和目标显示位置的距离
        double distance = QLineF(pos, targetDisplayPos).length();

        if (distance <= 5.0)   // 5像素容差
        {
            return mb->pihao;
        }
    }
    return -1;
}

int PPIGraphicsItem::getCurrentTargetLabel(const QPointF &pos)
{
    QList<int> keyList = m_rhkq.keys();
    for (int pihao : keyList) {
        QSharedPointer<Mubiao> mb = m_rhkq.value(pihao);
        if (!mb || mb->daji_flag)
            continue;

        // 获取目标航迹（物理坐标，米）
        QVector<QPointF> trajectoryMeters = mb->circleppi_hangji.get_data();
        if (trajectoryMeters.isEmpty())
            continue;

        // 计算目标位置
        QPointF  meterPt = trajectoryMeters.last();
        float    fw;
        uint32_t jl;
        MubiaoAdapter::cartesianToPolar(meterPt.x(), meterPt.y(), fw, jl);
        QPointF targetDisplayPos = MubiaoAdapter::polarToCartesian(fw, jl, 0.0, 0.0, m_radius, m_huan_ju);

        // 计算标牌位置
        QPointF labelPos = targetDisplayPos + mb->labelOffset;

        // 计算标牌尺寸（与paintTargets中的计算保持一致）
        QString labelText = QString::number(mb->pihao);
        QFont   labelFont;
        labelFont.setPointSize(9);
        labelFont.setBold(true);
        QFontMetricsF fm(labelFont);
        QRectF        textRect = fm.boundingRect(labelText);
        QRectF        bgRect   = textRect.adjusted(-4, -2, 4, 2);
        bgRect.moveCenter(labelPos);

        // 检查点击位置是否在标牌矩形内
        if (bgRect.contains(pos)) {
            return mb->pihao;
        }
    }
    return -1;
}

void PPIGraphicsItem::setRadius(double radius)
{
    if (radius < 100)
        radius = 100;
    if (radius > 3000)
        radius = 3000;

    m_radius = radius;
    updateRulerBufferSize();
    m_needRedrawRuler = true;
    prepareGeometryChange();
    update();
}

void PPIGraphicsItem::setHuanJu(uint32_t huanJu)
{
    m_huan_ju         = huanJu;
    m_needRedrawRuler = true;
    update();
}

void PPIGraphicsItem::setPPIOpacity(double opacity)
{
    if (opacity < 0.0)
        opacity = 0.0;
    if (opacity > 1.0)
        opacity = 1.0;

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
    }
    else {
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
    if (need) {
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
void PPIGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_lastClickPos = event->pos();

    if (m_draggable) {
        // 拖动模式：改变鼠标样式并调用基类处理
        setCursor(QCursor(Qt::ClosedHandCursor));
        QGraphicsItem::mousePressEvent(event);
    }
    else {
        // 非拖动模式：检测是否点击了标牌
        int labelId = getCurrentTargetLabel(event->pos());
        if (labelId > 0) {
            // 点击了标牌，进入标牌拖动模式
            m_draggingLabelId = labelId;
            setCursor(QCursor(Qt::ClosedHandCursor));
            event->accept();
        }
        else {
            // 未点击标牌，正常处理
            event->accept();
        }
    }
}

void PPIGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF pos      = event->pos();
    int     targetId = getCurrentTarget(pos);

    if (targetId > 0) {
        // 标记为双击事件，阻止后续的单击事件处理
        m_isDoubleClick = true;
        emit targetDoubleClicked(targetId);
    }

    QGraphicsItem::mouseDoubleClickEvent(event);
}

void PPIGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF pos = event->pos();

    if (m_draggable) {
        // 拖动模式：恢复鼠标样式
        setCursor(QCursor(Qt::OpenHandCursor));
        QGraphicsItem::mouseReleaseEvent(event);
    }
    else if (m_draggingLabelId > 0) {
        // 标牌拖动模式：结束拖动
        m_draggingLabelId = -1;
        setCursor(QCursor(Qt::ArrowCursor));
        event->accept();
    }
    else {
        // 检查是否是双击的一部分
        if (m_isDoubleClick) {
            // 是双击的一部分，重置标志并跳过单击处理
            m_isDoubleClick = false;
            event->accept();
            return;
        }

        // 非拖动模式：检测是否点击了目标
        if (QLineF(pos, m_lastClickPos).length() < 5.0)   // 判断是点击而非拖拽
        {
            int targetId = getCurrentTarget(pos);
            if (targetId > 0) {
                emit targetClicked(targetId);
            }
        }
        event->accept();
    }
}

void PPIGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_draggable) {
        // 拖动模式：调用基类处理移动
        QGraphicsItem::mouseMoveEvent(event);
    }
    else if (m_draggingLabelId > 0) {
        // 标牌拖动模式：更新标牌偏移
        QSharedPointer<Mubiao> mb = m_rhkq.value(m_draggingLabelId);
        if (mb) {
            // 获取目标当前位置
            QVector<QPointF> trajectoryMeters = mb->circleppi_hangji.get_data();
            if (!trajectoryMeters.isEmpty()) {
                QPointF  meterPt = trajectoryMeters.last();
                float    fw;
                uint32_t jl;
                MubiaoAdapter::cartesianToPolar(meterPt.x(), meterPt.y(), fw, jl);
                QPointF targetDisplayPos = MubiaoAdapter::polarToCartesian(fw, jl, 0.0, 0.0, m_radius, m_huan_ju);

                // 计算新的偏移量
                mb->labelOffset = event->pos() - targetDisplayPos;

                // 触发重绘
                update();
            }
        }
        event->accept();
    }
    else {
        // 非拖动模式：不处理移动
        event->accept();
    }
}

void PPIGraphicsItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    // 滚轮缩放PPI半径
    QPoint numDegrees = event->delta() > 0 ? QPoint(0, 120) : QPoint(0, -120);

    if (numDegrees.y() > 0) {
        setRadius(m_radius + 20);
    }
    else {
        setRadius(m_radius - 20);
    }

    event->accept();
}

void PPIGraphicsItem::updateRulerBufferSize()
{
    // 根据半径动态调整缓存大小
    int bufferSize = static_cast<int>(m_radius * 2 + 200);   // 留出边距
    if (bufferSize < 800)
        bufferSize = 800;
    if (bufferSize > 6400)
        bufferSize = 6400;

    m_rulerBuffer = QPixmap(bufferSize, bufferSize);
    m_rulerBuffer.fill(Qt::transparent);
    m_needRedrawRuler = true;
}

void PPIGraphicsItem::toggleTargetFocus(int targetId)
{
    QSharedPointer<Mubiao> mb = m_rhkq.value(targetId);
    if (!mb)
        return;

    // 切换重点关注状态
    mb->zhongdian = !mb->zhongdian;

    // 发出状态切换信号
    emit targetFocusToggled(targetId, mb->zhongdian);

    // 触发重绘
    update();
}

void PPIGraphicsItem::toggleTargetGuidance(int targetId)
{
    QSharedPointer<Mubiao> mb = m_rhkq.value(targetId);
    if (!mb)
        return;

    // 切换导引状态
    mb->daoyin_flag = !mb->daoyin_flag;
    qDebug() << "current daoyin id: " << targetId << "daoyin_flag" << mb->daoyin_flag << endl;
    // 如果开始导引，自动标记为重点关注
    if (mb->daoyin_flag && !mb->zhongdian) {
        mb->zhongdian = true;
        emit targetFocusToggled(targetId, true);
    }

    // 发出导引状态切换信号
    emit targetGuidanceToggled(targetId, mb->daoyin_flag);

    // 触发重绘
    update();
}
