 # 定义一个库 (Library)，而不是可执行文件
 # STATIC 表示静态库 (.lib/.a)
 add_library(Service STATIC
    UavModel.cpp
    UavModel.h
    flightpath.cpp
    flightpath.h
    trajectorygenerator.cpp
    trajectorygenerator.h
    simulationmanager.cpp
    simulationmanager.h
)

 # 这里只是一个“纯逻辑”核心模块，目前没有直接使用 Qt 类型，
 # 如果以后在 Service 里用到 Qt（比如 QString、QVector 等），
 # 可以再在顶层统一 find_package 并在这里链接。

 # 关键点：设置头文件搜索路径
 # PUBLIC 意味着：
 # 1. 编译 Service 模块自己时，能找到当前目录 (.)
 # 2. 任何依赖 Core 的其他模块 (比如 Gui)，也会自动获得这个搜索路径
 target_include_directories(Service PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
 target_link_libraries(Service PRIVATE Qt5::Core Qt5::Gui)
