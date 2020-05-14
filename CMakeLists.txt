cmake_minimum_required(VERSION 3.15)
project(taiko_tool)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(Boost REQUIRED COMPONENTS iostreams)
#configure_file(assets/template.nus3bank ${CMAKE_BINARY_DIR}/template.nus3bank COPYONLY)

set(QT_USE_QTMULTIMEDIA TRUE)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS forms)

add_executable(taiko_tool "")
target_include_directories(taiko_tool PUBLIC ./src/include)
add_subdirectory(src)
add_subdirectory(forms)
add_subdirectory(assets)

target_link_libraries(taiko_tool Qt5::Widgets Boost::iostreams)
qt5_use_modules(taiko_tool Multimedia)
