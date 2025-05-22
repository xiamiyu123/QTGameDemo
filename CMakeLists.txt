cmake_minimum_required(VERSION 3.10)
project(ScrollingTerrain LANGUAGES CXX)

# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用 Qt 自动化工具
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 若未指定构建类型，默认为 Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# 不同构建类型下的编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-Wall -Wextra)
else()
    add_compile_options(-O2)
endif()

# 查找所需的 Qt6 模块
find_package(Qt6 COMPONENTS Core Gui Widgets Svg OpenGL OpenGLWidgets Network Multimedia MultimediaWidgets Concurrent REQUIRED)

# 源文件与头文件
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS include/*.h)

# 资源目录与文件列表
set(IMAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resource")
file(GLOB_RECURSE IMAGE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${IMAGE_DIR}/*")

# 生成 qrc 文件路径
set(GENERATED_QRC "${CMAKE_BINARY_DIR}/auto_resources.qrc")

# 构建 qrc 内容到变量
set(QRC_CONTENT "<RCC>\n  <qresource prefix=\"/\">\n")
foreach(img IN LISTS IMAGE_FILES)
    string(APPEND QRC_CONTENT "    <file>${img}</file>\n")
endforeach()
string(APPEND QRC_CONTENT "  </qresource>\n</RCC>\n")

# 仅当内容变化时才写入 qrc 文件，以避免重复触发资源编译
if(EXISTS ${GENERATED_QRC})
    file(READ ${GENERATED_QRC} OLD_QRC_CONTENT)
else()
    set(OLD_QRC_CONTENT "")
endif()

if(NOT QRC_CONTENT STREQUAL OLD_QRC_CONTENT)
    file(WRITE ${GENERATED_QRC} "${QRC_CONTENT}")
endif()

# 添加 Qt 资源
qt6_add_resources(QRC_SOURCES ${GENERATED_QRC})

# 生成可执行文件
add_executable(${PROJECT_NAME}
        ${SOURCES}
        ${HEADERS}
        ${QRC_SOURCES}
)

# Release 模式下的编译与链接优化
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL /Gy /Oi /Ot /Ox")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ffast-math")
endif()

file(COPY ${CMAKE_SOURCE_DIR}/resource DESTINATION ${CMAKE_BINARY_DIR})

# 编译定义
target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
        $<$<CONFIG:Release>:QT_MESSAGELOGCONTEXT>
        QT_USE_QSTRINGBUILDER
        QT_FORCE_ASSERTS
)

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接 Qt 库
target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Svg
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        Qt6::Network
        Qt6::Multimedia
        Qt6::MultimediaWidgets
        Qt6::Concurrent
)
