/**
 * @file LockedHash.h
 * @brief 线程安全的哈希表类头文件
 * @details 提供线程安全的QHash封装，支持多线程并发访问
 *          使用智能指针管理值对象，自动处理内存管理
 */

#ifndef LOCKEDHASH_H
#define LOCKEDHASH_H

#pragma once
#include <QHash>
#include <QReadWriteLock>
#include <QSharedPointer>

/**
 * @class LockedHash
 * @brief 线程安全的哈希表模板类
 * @details 使用读写锁保护，支持多线程并发访问
 *          使用智能指针管理值对象，自动处理内存管理
 *          设计原因：
 *          1. 线程安全：使用QReadWriteLock确保多线程环境下的数据安全
 *          2. 智能指针：使用QSharedPointer管理值对象，避免内存泄漏
 *          3. 函数式修改：提供modify方法，支持函数式编程风格
 *          4. 性能优化：读写锁允许多个读操作并发，提升读性能
 *          提升：
 *          - 安全性：支持多线程并发访问，避免数据竞争
 *          - 内存管理：智能指针自动管理内存，避免内存泄漏
 *          - 性能：读写锁优化，读操作可并发执行
 *          - 易用性：提供函数式修改接口，简化代码
 * @tparam T 哈希表值类型
 */
template <typename T>
class LockedHash
{
public:
    /**
     * @brief 读写锁对象
     * @details 使用mutable修饰，允许在const方法中加锁
     *          提升：支持const方法的安全访问，提升接口设计
     */
    mutable QReadWriteLock lock;

    /**
     * @brief 检查是否包含指定键
     * @param key 要查找的键
     * @return 如果包含该键返回true，否则返回false
     * @details 使用读锁保护，支持多线程并发读取
     */
    bool contains(int key) const
    {
        QReadLocker locker(&lock);
        return data.contains(key);
    }

    /**
     * @brief 获取指定键对应的值
     * @param key 要查找的键
     * @return 值的智能指针，如果不存在返回空指针
     * @details 使用读锁保护，支持多线程并发读取
     *          返回智能指针，自动管理内存
     *          提升：避免内存泄漏，提升代码安全性
     */
    QSharedPointer<T> value(int key) const
    {
        QReadLocker locker(&lock);
        return data.value(key);
    }

    /**
     * @brief 获取所有键的列表
     * @return 所有键的列表
     * @details 使用读锁保护，支持多线程并发读取
     *          返回键的拷贝，确保数据安全
     */
    QList<int> keys() const
    {
        QReadLocker locker(&lock);
        return data.keys();
    }

    /**
     * @brief 预分配容量
     * @param size 要预分配的容量
     * @details 使用写锁保护，预分配哈希表容量
     *          提升：减少哈希表扩容次数，提升性能
     */
    void reserve(int size)
    {
        QWriteLocker locker(&lock);
        data.reserve(size);
    }

    /**
     * @brief 获取哈希表大小
     * @return 哈希表中元素的数量
     * @details 使用读锁保护，支持多线程并发读取
     */
    int size() const
    {
        QReadLocker locker(&lock);
        return data.size();
    }

    /**
     * @brief 插入键值对
     * @param key 键
     * @param value 值的智能指针
     * @details 使用写锁保护，确保线程安全
     *          如果键已存在，会覆盖旧值
     *          提升：支持并发插入，提升系统性能
     */
    void insert(int key, QSharedPointer<T> value)
    {
        QWriteLocker locker(&lock);
        data.insert(key, value);
    }

    /**
     * @brief 移除指定键的键值对
     * @param key 要移除的键
     * @details 使用写锁保护，确保线程安全
     *          提升：支持并发删除，提升系统性能
     */
    void remove(int key)
    {
        QWriteLocker locker(&lock);
        data.remove(key);
    }

    /**
     * @brief 清空哈希表
     * @details 移除所有键值对，使用写锁保护
     *          提升：支持容器重置功能，提升系统灵活性
     */
    void clear()
    {
        QWriteLocker locker(&lock);
        data.clear();
    }

    /**
     * @brief 修改指定键对应的值
     * @param key 要修改的键
     * @param func 修改函数，接受T&类型的引用参数
     * @details 使用写锁保护，确保线程安全
     *          如果键不存在，不执行任何操作
     *          设计原因：提供函数式修改接口，支持原子性修改
     *          提升：避免先读取再写入的竞态条件，提升数据一致性
     * @tparam Functor 函数对象类型
     */
    template <typename Functor>
    void modify(int key, Functor func)
    {
        QWriteLocker locker(&lock);
        // 检查键是否存在
        if (data.contains(key))
        {
            // 调用函数对象修改值
            // 提升：支持任意修改逻辑，提升代码灵活性
            func(*data[key]);
        }
    }

    /**
     * @brief 修改所有值
     * @param func 修改函数，接受T&类型的引用参数
     * @details 使用写锁保护，确保线程安全
     *          遍历所有键值对，对每个值执行修改函数
     *          设计原因：支持批量修改，提升操作效率
     *          提升：原子性批量修改，避免逐个修改的竞态条件
     * @tparam Functor 函数对象类型
     */
    template <typename Functor>
    void modifyAll(Functor func)
    {
        QWriteLocker locker(&lock);
        // 遍历所有值，对每个值执行修改函数
        for (auto& item : data)
        {
            // 检查智能指针是否有效
            if (item)
                func(*item);
        }
    }

private:
    /**
     * @brief 实际存储数据的QHash容器
     * @details 键为int类型，值为T类型的智能指针
     *          使用QHash而非std::unordered_map，与Qt框架集成更好
     *          提升：利用Qt容器的优化，提升性能
     */
    QHash<int, QSharedPointer<T>> data;
};

#endif // LOCKEDHASH_H
