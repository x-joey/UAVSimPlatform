cmake_minimum_required(VERSION 3.16)

# 定义项目名称和版本
project(UAVSimPlatform VERSION 0.1 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置Qt路径（使用Windows上已安装的Qt 5.14.2）
# 注意：在WSL中访问Windows文件系统，使用/mnt/盘符/路径
#set(CMAKE_PREFIX_PATH "/mnt/d/software/qt/5.14.2/mingw73_64")

# 开启 Qt 自动处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 统一在顶层查找 Qt5 所需模块，子目录只做链接即可
find_package(Qt5 REQUIRED COMPONENTS
    Core
    Widgets
    Concurrent
    Sql
    Location
    Positioning
    Quick
    QuickWidgets
)

# 添加子模块 (顺序：先 Core, Service 再 Gui)
add_subdirectory(Core)
add_subdirectory(Service)
add_subdirectory(Gui)
