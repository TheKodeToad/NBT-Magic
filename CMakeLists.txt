cmake_minimum_required(VERSION 3.25)

project(nbt-magic VERSION 0.1)
set(nbt-magic_NAME "NBT Magic")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

configure_file(info.hpp.in info.hpp)

add_executable(${PROJECT_NAME}
        main.cpp
        info.hpp

        nbt/io.hpp
        nbt/io.cpp
        nbt/tag.hpp
        editor_window.hpp
        editor_window.cpp
        tag_model.hpp
        tag_model.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
add_compile_options(-fno-inline-functions -O0)