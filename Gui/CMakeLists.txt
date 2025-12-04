 # 这里只负责 GUI，可执行程序在这里定义

 # 定义可执行文件 (Executable)
 add_executable(UavSimApp
     main.cpp
     MainWindow.cpp
     MainWindow.h
     pathitem.cpp
     pathitem.h
     uavitem.cpp
     uavitem.h
    uavlabelitem.cpp
    uavlabelitem.h
     simview.cpp
     simview.h
     simscene.cpp
     simscene.h
 )

 # 核心步骤：将 Gui 链接到 Core 库，以及需要的 Qt 模块
 # Qt 模块统一在顶层 root CMakeLists 里 find_package，这里只做链接
 target_link_libraries(UavSimApp PRIVATE
     Qt5::Widgets
     Qt5::Core
     Qt5::Concurrent
     Qt5::Sql
     Service
 )
