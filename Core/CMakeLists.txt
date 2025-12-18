# 新增 Core/CMakeLists.txt
add_library(Core STATIC
    Mubiao.h
    LockedHash.h
    LockedVector.h
    MubiaoAdapter.h
    MubiaoAdapter.cpp
    PPIDataManager.h
    PPIDataManager.cpp
    CoordinateConverter.h
    CoordinateConverter.cpp
)

# 设置头文件搜索路径
target_include_directories(Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Service
)

# 链接Qt模块
target_link_libraries(Core PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Positioning
)
