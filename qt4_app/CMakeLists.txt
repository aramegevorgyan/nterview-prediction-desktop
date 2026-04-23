cmake_minimum_required(VERSION 3.16)

project(interview_predictor_viewer LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(APP_SOURCES
    main.cpp
    mainwindow.cpp
)

set(APP_HEADERS
    mainwindow.h
)

add_executable(${PROJECT_NAME}
    ${APP_SOURCES}
    ${APP_HEADERS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
)
