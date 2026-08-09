qt_add_executable(MediaFlyouts WIN32
    main.cpp
    nowplaying.h nowplaying.cpp
    albumartprovider.h albumartprovider.cpp
)

qt_add_qml_module(MediaFlyouts
    URI MediaFlyouts
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

target_link_libraries(MediaFlyouts PRIVATE
    Qt6::Gui
    Qt6::Quick
    mediaflyout_core
)

if (WIN32)
    target_sources(MediaFlyouts PRIVATE winmediaprovider.h winmediaprovider.cpp)
    target_compile_definitions(MediaFlyouts PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    target_link_libraries(MediaFlyouts PRIVATE WindowsApp.lib)
endif()

