cmake_minimum_required(VERSION 3.16)

# 定义项目名称和版本
project(UAVSimPlatform VERSION 0.1 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 如果你用的是 Qt5，请确保 Kit 里选的是 Qt5，对应 MSVC/MinGW 要和编译器一致
# 如果 Qt 安装目录比较特别，可以在 Qt Creator 的 Kit 里设置 CMAKE_PREFIX_PATH，
# 或者在这里手动指定（不推荐同时两边都设，以免冲突）：
# set(CMAKE_PREFIX_PATH "C:/Qt/5.15.2/msvc2019_64")

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
)

# 添加子模块 (顺序：先 Service 再 Gui)
add_subdirectory(Service)
add_subdirectory(Gui)
