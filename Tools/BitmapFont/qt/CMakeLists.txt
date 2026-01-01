cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(BitmapFontConverter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

qt_add_executable(BitmapFontConverter
    main.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
)

set_property(TARGET BitmapFontConverter PROPERTY AUTOUIC ON)
set_property(TARGET BitmapFontConverter PROPERTY AUTOMOC ON)
set_property(TARGET BitmapFontConverter PROPERTY AUTORCC ON)

target_link_libraries(BitmapFontConverter PRIVATE Qt6::Widgets)
