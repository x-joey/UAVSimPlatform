#ifndef CIRCLEPPIWIDGET_H
#define CIRCLEPPIWIDGET_H

#include "mbwigt_bp.h"
#include "mubiao_xingzhi.h"
#include "mubiao.h"
#include "cz_data.h"
#include <QCheckBox>
#include <QLabel>
#include <QObject>
#include <QPainter>
#include <QPushButton>
#include <QTimer>
#include <QWidget>
#include <QMutex>
class CirclePPIWidget : public QWidget
{
    Q_OBJECT
public:
    CirclePPIWidget(QWidget* parent = nullptr);
    ~CirclePPIWidget();

    void   QingChu_WeiJi();
    void   GradientArc(int type, QPainter* painter, double start, double stop);
    void   drawDashedLine(QPainter* painter, double fangwei);
    void   quxiao_pihao_fun(int pihao);
    int    get_Current_MuBiao(QPoint clickPos);
    int    calc_zhongxinfw(QPoint clickPos);
    void   drawSelectionRect(QPainter& painter);  // 绘制框选区域
    QPoint calculateCenterPoint();                // 计算框选区域的中心位置
    void   paintruler(QPainter* painter);
    void   huan_ju_cal(uint8_t num);
    void   setNeedRedrawRuler(bool needRedraw);

    void draw_mubiao(QSharedPointer<Mubiao> mubiao, QPainter* painter);

    QPointF get_xyz(float fw, float gd, uint32_t jl);
    void generateTestTargets();
    void updateTestDianJi();
    void generateTestDianJi();
    QHash<int, QSharedPointer<MBWigt_BP>> Widg_bp_list;     // 标牌数据
    bool                                  m_isSelecting;    // 是否正在框选
    QVector<int>                          m_selectedPihao;  // 存储选中批号

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
    void resizeEvent(QResizeEvent* event);

signals:
    void create_jinshe(double left, double right, double top, double bottom);

private slots:
    void PPIGuiLing_clicked();
    void JuBiaoJian_clicked();
    void JuBiaoJia_clicked();
    void asyncRender();
    void mouseclick();

private:
    QTimer*      timer;
    QPixmap      BeiXiang;
    QLabel*      lbl_jubiao;
    QPushButton* ptn_fuwei;
    QPushButton* ptn_jubiaojia;
    QPushButton* ptn_jubiaojian;

    uint64_t time_count = 0;

    QCheckBox*      chk_kuangxuan;
    MuBiao_XingZhi* mubiaoxingzhi;
    QPoint          m_startPoint;  // 框选区域的起始点
    QPoint          m_endPoint;    // 框选区域的结束点
    int             max_num = 120;
    QPixmap         m_backBuffer;
    QPixmap         m_rulerBuffer;             // 用于缓存 paintruler 函数的绘制结果
    bool            m_needRedrawRuler = true;  // 标志位，判断是否需要重新绘制标尺

    // 新增成员
    QThread* m_renderThread;
    QPixmap  m_frontBuffer;
    QMutex   m_bufferMutex;

    QTimer m_clickTimer;
};

#endif  // CIRCLEPPIWIDGET_H
