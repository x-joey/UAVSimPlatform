/**
 * @file simscene.h
 * @brief 仿真场景类头文件
 * @details 自定义图形场景，负责绘制网格背景和处理用户交互
 */

#ifndef SIMSCENE_H
#define SIMSCENE_H
#include <QGraphicsScene>
#include <QPainter>
#include <QRectF>

/**
 * @class SimScene
 * @brief 仿真场景类
 * @details 自定义场景，负责绘制网格背景，并在点击图元时发出信号
 *          设计原因：
 *          1. 自定义背景：绘制网格背景，提升可视化效果
 *          2. 事件处理：识别用户点击的图元，发出相应信号
 *          3. 性能优化：只绘制可见区域，提升渲染性能
 *          提升：
 *          - 可视化：网格背景便于定位和观察
 *          - 交互性：支持点击选择，提升用户体验
 *          - 性能：只绘制可见区域，减少不必要的绘制
 */
class SimScene : public QGraphicsScene
{
    Q_OBJECT
public:
    /**
     * @brief 使用基类构造函数
     * @details 继承QGraphicsScene的所有构造函数
     */
    using QGraphicsScene::QGraphicsScene;

    /**
     * @brief 设置是否显示网格背景
     * @param show 是否显示网格
     * @details 控制网格背景的显示/隐藏，并触发场景更新
     *          提升：支持动态切换显示模式，提升灵活性
     */
    void setShowGrid(bool show)
    {
        m_showGrid = show;
        update();
    }

    /**
     * @brief 获取网格显示状态
     * @return 如果显示网格返回true，否则返回false
     */
    bool showGrid() const { return m_showGrid; }

signals:
    /**
     * @brief 无人机点击信号
     * @param uavId 被点击的无人机ID
     * @details 当用户点击某个无人机图元时发出
     *          提升：使用信号槽机制，实现松耦合的组件通信
     */
    void uavClicked(int uavId);

protected:
    /**
     * @brief 重写背景绘制函数
     * @param painter 画笔对象
     * @param rect 当前视图可见的区域
     * @details 绘制网格背景，只绘制可见区域以优化性能
     *          设计原因：Qt只会让我们画屏幕上能看到的部分，优化性能
     *          提升：减少不必要的绘制，提升渲染性能
     */
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    /**
     * @brief 处理鼠标点击事件
     * @param event 鼠标事件对象
     * @details 识别被点击的图元（UavItem 或 UavLabelItem），发出相应信号
     *          提升：支持用户交互，提升用户体验
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    /**
     * @brief 是否显示网格
     * @details 控制网格背景的显示状态，默认显示
     *          提升：支持动态切换，适应不同显示模式
     */
    bool m_showGrid = false;
};

#endif   // SIMSCENE_H
