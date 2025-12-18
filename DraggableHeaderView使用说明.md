# DraggableHeaderView 使用说明

## 概述

`DraggableHeaderView` 是一个可重用的Qt组件，实现了类似Excel的列拖动功能。该组件可以在任何Qt项目中使用。

## 特性

- ✅ **可视化拖动**：实时显示拖动列的高亮和插入位置指示器
- ✅ **平滑交互**：鼠标样式自动切换，提供良好的用户体验
- ✅ **完整兼容**：与QTableWidget完美集成，保持所有原有功能
- ✅ **可配置**：可随时启用/禁用拖动功能
- ✅ **信号通知**：提供`columnSwapped`信号，方便保存用户的列顺序设置
- ✅ **独立组件**：只需2个文件，易于集成到其他项目

## 快速开始

### 1. 基本使用

```cpp
#include "DraggableHeaderView.h"

// 创建表格
QTableWidget *table = new QTableWidget();

// 创建可拖动表头并应用到表格
DraggableHeaderView *header = new DraggableHeaderView(Qt::Horizontal, table);
table->setHorizontalHeader(header);

// 就这么简单！现在表格的列可以拖动了
```

### 2. 启用/禁用拖动

```cpp
// 启用列拖动（默认已启用）
header->setColumnDraggingEnabled(true);

// 禁用列拖动
header->setColumnDraggingEnabled(false);

// 检查状态
bool enabled = header->isColumnDraggingEnabled();
```

### 3. 监听列交换事件

```cpp
// 连接信号以保存用户的列顺序偏好
connect(header, &DraggableHeaderView::columnSwapped,
        this, [](int oldIndex, int newIndex) {
    qDebug() << "用户交换了列:" << oldIndex << "和" << newIndex;

    // 可以在这里保存列顺序到配置文件
    // saveColumnOrder(oldIndex, newIndex);
});
```

## 集成到其他项目

### 方法1：直接复制文件

1. 复制以下2个文件到你的项目：
   - `DraggableHeaderView.h`
   - `DraggableHeaderView.cpp`

2. 在CMakeLists.txt中添加：
   ```cmake
   add_executable(YourApp
       # ... 其他文件 ...
       DraggableHeaderView.cpp
       DraggableHeaderView.h
   )
   ```

3. 在你的代码中包含头文件：
   ```cpp
   #include "DraggableHeaderView.h"
   ```

### 方法2：作为静态库

如果在多个项目中使用，可以编译为静态库：

```cmake
# 创建静态库
add_library(DraggableHeaderView STATIC
    DraggableHeaderView.cpp
    DraggableHeaderView.h
)

target_link_libraries(DraggableHeaderView PUBLIC
    Qt5::Widgets
)

# 在其他项目中链接
target_link_libraries(YourApp PRIVATE
    DraggableHeaderView
)
```

## API 文档

### 构造函数

```cpp
DraggableHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr)
```
- **orientation**: 表头方向（`Qt::Horizontal` 或 `Qt::Vertical`）
- **parent**: 父控件（通常是QTableWidget）

### 公共方法

```cpp
void setColumnDraggingEnabled(bool enable)
```
启用或禁用列拖动功能。

```cpp
bool isColumnDraggingEnabled() const
```
返回当前是否启用列拖动。

### 信号

```cpp
void columnSwapped(int oldIndex, int newIndex)
```
当用户成功交换两列时发出此信号。
- **oldIndex**: 被拖动列的原始索引
- **newIndex**: 目标列的索引

## 使用示例

### 示例1：完整的表格设置

```cpp
#include <QApplication>
#include <QTableWidget>
#include "DraggableHeaderView.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 创建表格
    QTableWidget *table = new QTableWidget(10, 5);

    // 设置表头
    QStringList headers;
    headers << "ID" << "Name" << "Age" << "City" << "Status";
    table->setHorizontalHeaderLabels(headers);

    // 应用可拖动表头
    DraggableHeaderView *header = new DraggableHeaderView(Qt::Horizontal, table);
    table->setHorizontalHeader(header);

    // 填充一些示例数据
    for (int row = 0; row < 10; ++row) {
        table->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
        table->setItem(row, 1, new QTableWidgetItem(QString("User %1").arg(row + 1)));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(20 + row)));
        table->setItem(row, 3, new QTableWidgetItem("Beijing"));
        table->setItem(row, 4, new QTableWidgetItem("Active"));
    }

    table->show();
    return app.exec();
}
```

### 示例2：保存和恢复列顺序

```cpp
class MyWidget : public QWidget
{
    Q_OBJECT

public:
    MyWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        m_table = new QTableWidget(this);
        m_header = new DraggableHeaderView(Qt::Horizontal, m_table);
        m_table->setHorizontalHeader(m_header);

        // 监听列交换
        connect(m_header, &DraggableHeaderView::columnSwapped,
                this, &MyWidget::onColumnSwapped);

        // 恢复上次保存的列顺序
        restoreColumnOrder();
    }

private slots:
    void onColumnSwapped(int oldIndex, int newIndex)
    {
        // 保存列顺序到配置
        QSettings settings("MyCompany", "MyApp");
        QList<int> order = getCurrentColumnOrder();

        QVariantList orderVariant;
        for (int index : order) {
            orderVariant.append(index);
        }
        settings.setValue("columnOrder", orderVariant);
    }

private:
    QList<int> getCurrentColumnOrder()
    {
        QList<int> order;
        for (int i = 0; i < m_table->columnCount(); ++i) {
            order.append(m_header->logicalIndex(i));
        }
        return order;
    }

    void restoreColumnOrder()
    {
        QSettings settings("MyCompany", "MyApp");
        QVariantList orderVariant = settings.value("columnOrder").toList();

        // 根据保存的顺序重新排列列...
    }

    QTableWidget *m_table;
    DraggableHeaderView *m_header;
};
```

## 技术细节

### 实现原理

1. **拖动检测**：重写`mousePressEvent`检测用户按下表头
2. **移动跟踪**：通过`mouseMoveEvent`实时更新拖动位置
3. **可视反馈**：在`paintEvent`中绘制高亮和插入指示器
4. **数据交换**：在`mouseReleaseEvent`中实际交换列数据

### 性能优化

- 使用半透明绘制减少视觉干扰
- 只在拖动时重绘，避免不必要的开销
- 禁用排序时交换列，避免触发额外的排序操作

### 兼容性

- Qt 5.x 及以上版本
- 支持Windows、Linux、macOS
- 与QTableWidget、QTableView完美兼容

## 常见问题

### Q: 为什么拖动时列没有交换？

A: 请确保：
1. 使用`setHorizontalHeader()`正确设置了表头
2. 调用了`setColumnDraggingEnabled(true)`
3. 父控件是QTableWidget（其他控件可能需要额外适配）

### Q: 可以拖动行吗？

A: 可以！使用`Qt::Vertical`方向创建表头即可：
```cpp
DraggableHeaderView *vHeader = new DraggableHeaderView(Qt::Vertical, table);
table->setVerticalHeader(vHeader);
```

### Q: 如何在拖动后保持排序功能？

A: 组件会自动处理。在交换列时临时禁用排序，交换完成后恢复。

## 许可证

本组件作为UAVSimPlatform项目的一部分，可自由使用和修改。

## 贡献

欢迎提交Issue和Pull Request来改进这个组件！

## 更新日志

### v1.0 (2024-12-16)
- ✅ 初始版本发布
- ✅ 支持水平和垂直方向的列/行拖动
- ✅ 实时可视化反馈
- ✅ 完整的API和信号支持
