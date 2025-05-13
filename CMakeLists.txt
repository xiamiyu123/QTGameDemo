# CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(ScrollingTerrain LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Svg REQUIRED)

file(GLOB SOURCES
        src/*.cpp
)
file(GLOB HEADERS
        include/*.h
)

# 设置资源路径
set(IMAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resource")

# 获取该目录下的所有文件（可根据需要添加过滤器）
file(GLOB_RECURSE IMAGE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${IMAGE_DIR}/*)

# 生成.qrc文件路径
set(GENERATED_QRC "${CMAKE_BINARY_DIR}/auto_resources.qrc")

# 生成qrc文件内容
file(WRITE ${GENERATED_QRC} "<RCC>\n  <qresource prefix=\"/\">\n")
foreach(file IN LISTS IMAGE_FILES)
    file(APPEND ${GENERATED_QRC} "    <file>${file}</file>\n")
endforeach()
file(APPEND ${GENERATED_QRC} "  </qresource>\n</RCC>\n")

qt6_add_resources(QRC_SOURCES ${GENERATED_QRC})

# 在生成资源之前或之后都可
file(COPY ${IMAGE_DIR} DESTINATION ${CMAKE_BINARY_DIR})



add_executable(${PROJECT_NAME}
        ${SOURCES}
        ${HEADERS}
        ${QRC_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg
)

