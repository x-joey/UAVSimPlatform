// 线程安全的向量
#include <QReadWriteLock>
template <typename T>
class LockedVector
{
public:
    mutable QReadWriteLock lock;

    LockedVector(int maxSize = 100) : m_maxSize(maxSize) {}

    void append(const T& value)
    {
        QWriteLocker locker(&lock);
        if (data.size() >= m_maxSize)
        {
            data.removeFirst();
        }
        data.append(value);
    }

    QVector<T> get_data() const
    {
        QReadLocker locker(&lock);
        return data;
    }

    T last() const
    {
        QReadLocker locker(&lock);
        return data.isEmpty() ? T() : data.last();
    }

    void clear()
    {
        QWriteLocker locker(&lock);
        data.clear();
    }

    bool isEmpty() const
    {
        QReadLocker locker(&lock);
        return data.isEmpty();
    }

    int size() const
    {
        QReadLocker locker(&lock);
        return data.size();
    }

private:
    QVector<T> data;
    int m_maxSize;
};
