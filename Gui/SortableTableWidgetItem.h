/**
 * @file SortableTableWidgetItem.h
 * @brief 自定义排序表格项类
 * @details 通过重载operator<实现基于UserRole数据的自定义排序，
 *          而不是基于显示文本排序
 */

#ifndef SORTABLETABLEWIDGETITEM_H
#define SORTABLETABLEWIDGETITEM_H

#include <QTableWidgetItem>

/**
 * @class SortableTableWidgetItem
 * @brief 支持自定义排序的表格项
 * @details 重载operator<方法，优先使用UserRole数据进行排序
 *          如果UserRole未设置，则回退到文本排序
 */
class SortableTableWidgetItem : public QTableWidgetItem
{
public:
    /**
     * @brief 构造函数
     * @param text 显示文本
     */
    explicit SortableTableWidgetItem(const QString &text = QString())
        : QTableWidgetItem(text)
    {
    }

    /**
     * @brief 小于运算符重载
     * @param other 另一个表格项
     * @return 如果当前项小于other返回true，否则返回false
     * @details 优先使用UserRole数据排序，如果未设置则使用文本排序
     */
    bool operator<(const QTableWidgetItem &other) const override
    {
        // 获取两个项的UserRole数据
        QVariant thisData  = data(Qt::UserRole);
        QVariant otherData = other.data(Qt::UserRole);

        // 如果两个项都设置了UserRole数据，则使用UserRole数据排序
        if (thisData.isValid() && otherData.isValid()) {
            // 尝试转换为整数进行比较
            bool thisOk, otherOk;
            int  thisInt  = thisData.toInt(&thisOk);
            int  otherInt = otherData.toInt(&otherOk);

            if (thisOk && otherOk) {
                return thisInt < otherInt;
            }

            // 如果不是整数，尝试转换为浮点数
            double thisDouble  = thisData.toDouble(&thisOk);
            double otherDouble = otherData.toDouble(&otherOk);

            if (thisOk && otherOk) {
                return thisDouble < otherDouble;
            }

            // 如果都不行，转换为字符串比较
            return thisData.toString() < otherData.toString();
        }

        // 如果没有设置UserRole数据，回退到默认的文本排序
        return QTableWidgetItem::operator<(other);
    }
};

#endif   // SORTABLETABLEWIDGETITEM_H
