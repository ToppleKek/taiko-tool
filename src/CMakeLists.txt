target_sources(taiko_tool
    PRIVATE
        BuildTabManager.cpp
        main.cpp
        SongManager.cpp
        SongTabManager.cpp
        TaikoSong.cpp
        TaikoTool.cpp)

add_subdirectory(dialogs)
add_subdirectory(include)
