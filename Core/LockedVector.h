/**
 * @file LockedVector.h
 * @brief 线程安全的向量容器类头文件
 * @details 提供线程安全的QVector封装，支持多线程并发访问
 *          自动限制最大容量，超出时移除最旧元素，避免内存无限增长
 */

#ifndef LOCKEDVECTOR_H
#define LOCKEDVECTOR_H

#include <QReadWriteLock>
#include <QVector>

/**
 * @class LockedVector
 * @brief 线程安全的向量容器模板类
 * @details 使用读写锁保护，支持多线程并发访问
 *          自动限制最大容量，超出时移除最旧元素
 *          设计原因：
 *          1. 线程安全：使用QReadWriteLock确保多线程环境下的数据安全
 *          2. 自动限制：超出容量时自动移除最旧元素，实现FIFO队列
 *          3. 性能优化：读写锁允许多个读操作并发，提升读性能
 *          4. 内存管理：限制最大容量，避免内存无限增长
 *          提升：
 *          - 安全性：支持多线程并发访问，避免数据竞争
 *          - 性能：读写锁优化，读操作可并发执行
 *          - 内存：自动限制容量，保持系统性能稳定
 * @tparam T 容器元素类型
 */
template<typename T> class LockedVector
{
public:
    /**
     * @brief 读写锁对象
     * @details 使用mutable修饰，允许在const方法中加锁
     *          提升：支持const方法的安全访问，提升接口设计
     */
    mutable QReadWriteLock lock;

    /**
     * @brief 构造函数
     * @param maxSize 最大容量，默认100
     * @details 初始化容器，设置最大容量限制
     *          提升：自动限制容量，避免内存无限增长
     */
    LockedVector(int maxSize = 100)
        : m_maxSize(maxSize)
    {}

    /**
     * @brief 从另一个 LockedVector 复制数据
     * @param other 源容器
     * @details 线程安全地从另一个容器复制数据
     *          提升：支持容器间数据复制，解决拷贝赋值问题
     */
    void copyFrom(const LockedVector<T> &other)
    {
        // 获取源数据（加读锁）
        QVector<T> otherData;
        {
            QReadLocker otherLocker(&other.lock);
            otherData = other.data;
        }

        // 写入当前容器（加写锁）
        QWriteLocker locker(&lock);
        data = otherData;
    }


    /**
     * @brief 添加元素到容器末尾
     * @param value 要添加的元素
     * @details 如果容器已满，自动移除最旧的元素（FIFO策略）
     *          使用写锁保护，确保线程安全
     *          提升：自动管理容量，实现循环缓冲区功能
     */
    void append(const T &value)
    {
        QWriteLocker locker(&lock);
        // 如果超出容量，移除最旧的元素
        // 设计原因：实现FIFO队列，保持容器大小稳定
        // 提升：避免内存无限增长，保持系统性能
        if (data.size() >= m_maxSize) {
            data.removeFirst();
        }
        data.append(value);
    }

    /**
     * @brief 获取容器数据的拷贝
     * @return 容器数据的完整拷贝
     * @details 使用读锁保护，支持多线程并发读取
     *          返回拷贝而非引用，确保数据安全
     *          提升：避免外部修改影响内部数据，提升数据安全性
     *          注意：当数据量大时，拷贝可能影响性能
     */
    QVector<T> get_data() const
    {
        QReadLocker locker(&lock);
        return data;
    }

    /**
     * @brief 获取最后一个元素
     * @return 最后一个元素，如果容器为空返回默认构造值
     * @details 使用读锁保护，支持多线程并发读取
     *          提升：快速访问最新数据，无需拷贝整个容器
     */
    T last() const
    {
        QReadLocker locker(&lock);
        return data.isEmpty() ? T() : data.last();
    }

    /**
     * @brief 清空容器
     * @details 移除所有元素，使用写锁保护
     *          提升：支持容器重置功能，提升系统灵活性
     */
    void clear()
    {
        QWriteLocker locker(&lock);
        data.clear();
    }

    /**
     * @brief 检查容器是否为空
     * @return 如果容器为空返回true，否则返回false
     * @details 使用读锁保护，支持多线程并发读取
     */
    bool isEmpty() const
    {
        QReadLocker locker(&lock);
        return data.isEmpty();
    }

    /**
     * @brief 获取容器大小
     * @return 容器中元素的数量
     * @details 使用读锁保护，支持多线程并发读取
     */
    int size() const
    {
        QReadLocker locker(&lock);
        return data.size();
    }

private:
    /**
     * @brief 实际存储数据的QVector容器
     * @details 使用QVector而非std::vector，与Qt框架集成更好
     *          提升：利用Qt容器的优化，提升性能
     */
    QVector<T> data;

    /**
     * @brief 最大容量限制
     * @details 当容器大小达到此值时，新元素会替换最旧的元素
     *          提升：控制内存使用，保持系统性能稳定
     */
    int m_maxSize;
};

#endif   // LOCKEDVECTOR_H
