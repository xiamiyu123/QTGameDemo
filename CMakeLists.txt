cmake_minimum_required(VERSION 3.10)
project(ScrollingTerrain LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

set(SOURCES
    main.cpp
    mainwindow.cpp
    gamescene.cpp
    terraingenerator.cpp
    player.cpp
)

set(HEADERS
    mainwindow.h
    gamescene.h
    terraingenerator.h
    player.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

# 设置安装规则
install(TARGETS ${PROJECT_NAME} DESTINATION bin)