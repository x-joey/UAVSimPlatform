/**
 * @file UavModel.h
 * @brief 无人机模型类头文件
 * @details 表示仿真系统中的单个无人机实体，封装位置、航迹等状态信息
 *          采用数据类设计，专注于状态管理，提升代码可维护性
 */

#pragma once   // 防止头文件被重复引用，等同于 Java 的 import 机制保护

#include <QList>
#include <QPointF>
#include <QString>
#include <QVector>

/**
 * @class UavModel
 * @brief 无人机模型类
 * @details 表示仿真系统中的单个无人机实体，封装无人机的所有状态信息
 *          设计原因：
 *          1. 数据与逻辑分离：将无人机状态封装在独立类中，便于管理和扩展
 *          2. 值语义：使用基本类型和Qt容器，避免复杂的指针管理
 *          3. 不可变性：提供const getter方法，保护内部状态不被意外修改
 *          4. 轻量级：使用QVector存储航迹点，内存占用小，访问效率高
 *          提升：
 *          - 代码可读性和可维护性：清晰的数据结构，易于理解
 *          - 性能：使用Qt容器优化，支持快速随机访问
 *          - 扩展性：易于添加新的状态属性（如速度、高度等）
 */
class UavModel
{
public:
    /**
     * @brief 构造函数
     * @param id 无人机唯一标识符
     * @param name 无人机名称
     * @details 初始化无人机的基本属性，位置默认为(0,0)
     *          提升：使用初始化列表，避免默认构造后再赋值，提升性能
     */
    UavModel(int id, QString name);

    /**
     * @brief 更新无人机位置
     * @param stepIndex 当前仿真步数索引
     * @details 根据步数索引从航迹中获取对应位置并更新
     *          设计原因：
     *          1. 使用模运算防止数组越界，确保安全性
     *          2. 支持循环播放航迹，实现连续仿真
     *          提升：避免数组越界导致的程序崩溃，提升系统稳定性
     */
    void updatePosition(int stepIndex);

    /**
     * @brief 设置飞行航迹
     * @param path 航迹点序列
     * @details 设置无人机的完整飞行路径
     *          提升：支持动态更改航迹，增加仿真灵活性
     */
    void setFlightPath(const QVector<QPointF> &path);

    /**
     * @brief 设置当前航迹点索引
     * @param currentPoint 当前航迹点索引
     * @details 用于标记无人机当前在航迹中的位置
     *          提升：支持精确控制无人机位置，便于调试和回放
     */
    void setCurrentPoint(int currentPoint);

    /**
     * @brief 获取无人机ID
     * @return 无人机唯一标识符
     * @details 返回常量值，确保ID不可修改
     */
    int getId() const;

    /**
     * @brief 获取当前航迹点索引
     * @return 当前航迹点索引
     */
    int getCurrentPoint() const;

    /**
     * @brief 获取X坐标
     * @return X坐标值
     * @details 返回当前无人机在场景坐标系中的X坐标
     */
    double getX() const;

    /**
     * @brief 获取Y坐标
     * @return Y坐标值
     * @details 返回当前无人机在场景坐标系中的Y坐标
     */
    double getY() const;

    /**
     * @brief 获取无人机名称
     * @return 无人机名称字符串
     */
    QString getName() const;

    /**
     * @brief 获取航迹点序列
     * @return 航迹点向量的常量引用
     * @details 返回常量引用而非拷贝，避免不必要的内存复制
     *          提升：减少内存占用和拷贝开销，提升性能
     */
    const QVector<QPointF> &getPath() const;

private:
    /**
     * @brief 无人机唯一标识符
     * @details 用于区分不同的无人机实例
     */
    int m_id;

    /**
     * @brief 无人机名称
     * @details 用于显示和日志记录
     */
    QString m_name;

    /**
     * @brief 当前X坐标
     * @details 无人机在场景坐标系中的X坐标位置
     */
    double m_x;

    /**
     * @brief 当前Y坐标
     * @details 无人机在场景坐标系中的Y坐标位置
     */
    double m_y;

    /**
     * @brief 当前航迹点索引
     * @details 标识无人机当前处于航迹中的第几个点
     *          用于跟踪仿真进度和回放功能
     */
    int m_currentPoint;

    /**
     * @brief 存储已生成的航迹点序列
     * @details 使用QVector存储，支持快速随机访问
     *          提升：相比QList，QVector在连续内存中存储，缓存性能更好
     */
    QVector<QPointF> m_path;
};
