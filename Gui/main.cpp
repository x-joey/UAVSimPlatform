/**
 * @file main.cpp
 * @brief 应用程序入口文件
 * @details 创建Qt应用程序实例和主窗口，启动事件循环
 */

#include "MainWindow.h"
#include <QApplication>

/**
 * @brief 应用程序入口函数
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 应用程序退出码
 * @details 创建Qt应用程序实例和主窗口，启动事件循环
 *          程序流程：
 *          1. 创建QApplication实例，管理Qt应用程序的生命周期
 *          2. 创建MainWindow主窗口实例
 *          3. 显示主窗口
 *          4. 进入事件循环，等待用户交互和系统事件
 *          设计原因：
 *          - 使用Qt标准应用程序结构，确保正确的初始化和清理
 *          - 事件循环处理用户输入、定时器、信号槽等
 *          提升：
 *          - 代码规范性：遵循Qt应用程序标准结构
 *          - 资源管理：QApplication自动管理Qt资源
 */
int main(int argc, char *argv[])
{
    // 创建QApplication实例，管理Qt应用程序的生命周期
    // 提升：自动处理Qt框架的初始化、资源管理和清理
    QApplication a(argc, argv);
    
    // 创建主窗口实例
    // 提升：使用栈对象，自动管理内存，避免内存泄漏
    MainWindow w;
    
    // 显示主窗口
    // 提升：触发窗口显示和布局计算
    w.show();
    
    // 进入事件循环 (Event Loop)，等待用户交互和系统事件
    // 提升：处理用户输入、定时器、信号槽等事件，直到应用程序退出
    return a.exec();
}