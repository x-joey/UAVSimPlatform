#pragma once   // 防止头文件被重复引用，等同于 Java 的 import 机制保护

#include <QList>
#include <QPointF>
#include <QString>
#include <QVector>
// 这是一个表示“无人机”的数据类
class UavModel
{
public:
    // 构造函数
    UavModel(int id, QString name);

    // 模拟更新无人机位置 (示例业务逻辑)
    void updatePosition(int stepIndex);

    void setFlightPath(const QVector<QPointF> &path);

    // Getters
    int                     getId() const;
    double                  getX() const;
    double                  getY() const;
    QString                 getName() const;
    const QVector<QPointF> &getPath() const;

private:
    int     m_id;
    QString m_name;
    double  m_x;
    double  m_y;

    QVector<QPointF> m_path;   // 存储已生成的航迹点
};
