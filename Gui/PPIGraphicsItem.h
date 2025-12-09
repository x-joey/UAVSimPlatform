#ifndef PPIGRAPHICSITEM_H
#define PPIGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include "Mubiao.h"
#include "LockedHash.h"
#include "LockedVector.h"

/**
 * @brief PPI雷达显示图元（QGraphicsItem版本）
 * 集成CirclePPIWidget的所有显示功能到QGraphicsView架构
 */
class PPIGraphicsItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    explicit PPIGraphicsItem(QGraphicsItem* parent = nullptr);
    ~PPIGraphicsItem();

    // QGraphicsItem接口
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    // PPI参数设置
    void setRadius(double radius);
    double getRadius() const { return m_radius; }

    void setHuanJu(uint32_t huanJu);
    uint32_t getHuanJu() const { return m_huan_ju; }

    // 透明度控制
    void setPPIOpacity(double opacity);
    double getPPIOpacity() const { return m_opacity; }

    // 背景模式控制
    void setDrawBackground(bool draw) { m_drawBackground = draw; update(); }
    bool getDrawBackground() const { return m_drawBackground; }

    // 拖动控制
    void setDraggable(bool draggable);
    bool getDraggable() const { return m_draggable; }

    // 数据访问
    LockedHash<Mubiao>& getMubiaoHash() { return m_rhkq; }
    LockedVector<QVector<QPointF>>& getDianJiData() { return m_dj_point; }

    // 显示控制
    void setShowDianji(bool show) { m_show_dianji = show; update(); }
    bool getShowDianji() const { return m_show_dianji; }

    // 火力线控制
    void setHuoLiXianAngle(double angle) { m_HuoLiXian_MaPan = angle; update(); }
    double getHuoLiXianAngle() const { return m_HuoLiXian_MaPan; }

    // 扇形区域管理
    void addJinSheQuYu(double left, double right);
    void clearJinSheQuYu();

    // 刷新控制
    void setNeedRedrawRuler(bool need);

signals:
    void targetClicked(int targetId);
    void targetDoubleClicked(int targetId);

protected:
    // 事件处理
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void wheelEvent(QGraphicsSceneWheelEvent* event) override;

private slots:
    void onUpdateTimer();

private:
    // 绘制函数
    void paintRuler(QPainter* painter);              // 绘制标尺（圆环、刻度、方位盘）
    void paintTargets(QPainter* painter);            // 绘制目标
    void paintDianji(QPainter* painter);             // 绘制点迹
    void paintFireLine(QPainter* painter);           // 绘制火力线
    void paintSectors(QPainter* painter);            // 绘制扇形区域
    void drawGradientArc(QPainter* painter, int type, double start, double stop);  // 绘制渐变扇形

    // 工具函数
    QPointF polarToCartesian(float fangwei, uint32_t juli);  // 极坐标转笛卡尔
    int getCurrentTarget(const QPointF& pos);                 // 获取点击位置的目标
    void updateRulerBufferSize();                             // 更新标尺缓存大小

    // PPI参数
    double m_radius = 340;                  // PPI显示半径（像素）
    uint32_t m_huan_ju = 5000;              // 距离环距离（米，默认5km）
    int m_huanjianju = 0;                   // 圆环间距（像素，自动计算）

    // 显示控制
    double m_opacity = 1.0;                 // PPI透明度（0.0-1.0）
    bool m_drawBackground = true;           // 是否绘制黑色背景
    bool m_draggable = false;               // 是否可拖动

    // 数据容器
    LockedHash<Mubiao> m_rhkq;                        // 目标数据
    LockedVector<QVector<QPointF>> m_dj_point;        // 点迹数据

    // 扇形区域
    QVector<JinSheQuYu> m_JSQY_list;        // 禁射区域列表
    QVector<JinSheQuYu> m_FX_JS_list;       // 方向禁射列表
    QVector<JinSheQuYu> m_QY_JS_list;       // 区域禁射列表
    ZeRenShanQu m_BenDiZeRenShanQu;         // 责任扇区
    bool m_Is_ZeRenShanQu_Use = false;      // 是否使用责任扇区

    // 显示标志
    bool m_show_dianji = false;             // 是否显示点迹
    double m_HuoLiXian_MaPan = 90;          // 火力线角度

    // 缓存
    QPixmap m_rulerBuffer;                  // 标尺缓存
    bool m_needRedrawRuler = true;          // 是否需要重绘标尺

    // 定时器
    QTimer* m_updateTimer = nullptr;

    // 辅助变量
    QPointF m_lastClickPos;                 // 最后点击位置
    uint64_t m_time_count = 0;              // 时间计数
};

#endif // PPIGRAPHICSITEM_H
