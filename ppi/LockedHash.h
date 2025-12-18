#ifndef LOCKEDHASH_H
#define LOCKEDHASH_H

#pragma once
#include <QHash>
#include <QReadWriteLock>
#include <QSharedPointer>

template <typename T> class LockedHash
{
public:
    mutable QReadWriteLock lock;

    bool contains(int key) const
    {
        QReadLocker locker(&lock);
        return data.contains(key);
    }

    QSharedPointer<T> value(int key) const
    {
        QReadLocker locker(&lock);
        // 在锁保护下创建共享指针副本
        return QSharedPointer<T>(data.value(key));
    }

    QList<int> keys() const
    {
        QReadLocker locker(&lock);
        return data.keys();
    }

    void reserve(int size)
    {
        QWriteLocker locker(&lock);
        data.reserve(size);
    }

    int size() const
    {
        QReadLocker locker(&lock);
        return data.size();
    }

    void insert(int key, QSharedPointer<T> value)
    {
        QWriteLocker locker(&lock);
        data.insert(key, value);
    }

    void remove(int key)
    {
        QWriteLocker locker(&lock);
        data.remove(key);
    }

    void clear()
    {
        QWriteLocker locker(&lock);
        data.clear();
    }

    template <typename Functor> void modify(int key, Functor func)
    {
        QWriteLocker locker(&lock);
        if (data.contains(key))
        {
            func(*data[key]);
        }
    }

    template <typename Functor> void modifyAll(Functor func)
    {
        QWriteLocker locker(&lock);
        for (auto& item : data)
        {
            if (item)
                func(*item);
        }
    }

private:
    QHash<int, QSharedPointer<T>> data;
};

#endif  // LOCKEDHASH_H
