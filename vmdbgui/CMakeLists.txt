cmake_minimum_required(VERSION 3.7.0)

project(vmdbgui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets Core REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/../common)
include_directories(${PROJECT_SOURCE_DIR}/../virtualmachine)

add_executable(vmdbgui
    ${PROJECT_SOURCE_DIR}/../virtualmachine/vm.c
    src/mainwindow.cpp
    src/vmwrapper.cpp
    src/regmodel.cpp
    src/stackmodel.cpp
    src/instrmodel.cpp
    src/main.cpp
)

target_link_libraries(vmdbgui PRIVATE Qt5::Widgets)
