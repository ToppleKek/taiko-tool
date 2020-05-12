cmake_minimum_required(VERSION 3.15)
project(taiko_tool)

#option(R "Build vgmstream for song playback" OFF)
#
#if(USE_VGMSTREAM)
#    message("-- Building vgmstream--")
#    add_definitions(-DUSE_VGMSTREAM)
#    include(ExternalProject)
#    ExternalProject_Add(vgmstream GIT_REPOSITORY https://github.com/losnoco/vgmstream.git
#            GIT_TAG master
#            SOURCE_DIR ${CMAKE_BINARY_DIR}/vgmstream
#            BINARY_DIR ${CMAKE_BINARY_DIR}/vgmstream-build
#            INSTALL_DIR "${CMAKE_BINARY_DIR}/bin"
#            CMAKE_ARGS "-DBUILD_AUDACIOUS=OFF"
#            CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}")
#else()
#    message("vgmstream disabled: song playback will require you to specify the location of the vgmstream-cli binary")
#endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(Boost REQUIRED COMPONENTS iostreams)
find_package(ZLIB)
configure_file(assets/template.nus3bank ${CMAKE_BINARY_DIR}/template.nus3bank COPYONLY)

set(QT_USE_QTMULTIMEDIA TRUE)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS forms)

#add_executable(taiko_tool forms/TaikoTool.ui forms/SettingsDialog.ui assets/icons.qrc src/main.cpp src/TaikoTool.cpp src/GZip.h src/SongTabManager.cpp src/SongTabManager.h src/Utils.h src/SongManager.cpp src/SongManager.h src/InvalidJsonException.h src/dialogs/SettingsDialog.cpp src/dialogs/SettingsDialog.h src/BuildTabManager.cpp src/BuildTabManager.h src/TaikoSong.cpp src/TaikoSong.h)
add_executable(taiko_tool "")
target_include_directories(taiko_tool PUBLIC ./src/include)
add_subdirectory(src)
add_subdirectory(forms)
add_subdirectory(assets)

target_link_libraries(taiko_tool Qt5::Widgets Boost::iostreams ZLIB::ZLIB)
qt5_use_modules(taiko_tool Multimedia Concurrent)
