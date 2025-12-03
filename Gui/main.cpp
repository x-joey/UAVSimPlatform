#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv); // 管理 Qt 应用程序的生命周期
    MainWindow w;
    w.show(); // 显示窗口
    return a.exec(); // 进入事件循环 (Event Loop)，等待用户点击
}