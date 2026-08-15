cmake_minimum_required(VERSION 3.21)

project(MediaFlyouts VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Network Qml Quick Test Widgets)
qt_standard_project_setup(REQUIRES 6.5)

option(MEDIAFLYOUTS_BUILD_TESTS "Build the unit tests" ON)
if(MEDIAFLYOUTS_BUILD_TESTS)
    enable_testing()
endif()

# --- Model: cross-platform domain logic, shared by the app and the tests ---
qt_add_library(mediaflyout_core STATIC
    Model/FlyoutPositioner.h Model/FlyoutPositioner.cpp
    Model/TrackInfo.h        Model/TrackInfo.cpp
    Model/PlaybackController.h Model/PlaybackController.cpp
)
target_link_libraries(mediaflyout_core PUBLIC Qt6::Core)
target_include_directories(mediaflyout_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Model)

# --- Application executable ---
qt_add_executable(MediaFlyouts WIN32
    main.cpp
    Controllers/NowPlaying.h     Controllers/NowPlaying.cpp
    Controllers/TrayController.h Controllers/TrayController.cpp
    Controllers/FlyoutPlacement.h Controllers/FlyoutPlacement.cpp
    Controllers/SingleInstance.h Controllers/SingleInstance.cpp
    Model/AlbumArtProvider.h     Model/AlbumArtProvider.cpp
    Model/Settings.h             Model/Settings.cpp
)

target_include_directories(MediaFlyouts PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/Model
)

# Emit the runnable app (and its windeployqt runtime) into <root>/bin instead of the build tree.
set_target_properties(MediaFlyouts PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

set_source_files_properties(Views/Fluent.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(Views/Anim.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
# keep the font at a stable resource path regardless of its source folder
set_source_files_properties(Views/assets/fonts/FluentSystemIcons-Filled.ttf
    PROPERTIES QT_RESOURCE_ALIAS assets/fonts/FluentSystemIcons-Filled.ttf)

qt_add_qml_module(MediaFlyouts
    URI MediaFlyouts
    VERSION 1.0
    QML_FILES
        Views/App.qml
        Views/MediaFlyout.qml
        Views/VolumeFlyout.qml
        Views/LockKeysFlyout.qml
        Views/NextUpFlyout.qml
        Views/Dashboard.qml
        Views/TaskbarWidget.qml
        Views/Fluent.qml
        Views/Anim.qml
        Views/IconLabel.qml
        Views/SettingRow.qml
        Views/SettingCombo.qml
        Views/SettingSlider.qml
    RESOURCES
        Views/assets/fonts/FluentSystemIcons-Filled.ttf
)

target_link_libraries(MediaFlyouts PRIVATE
    Qt6::Gui
    Qt6::Network
    Qt6::Quick
    Qt6::Widgets
    mediaflyout_core
)

if (WIN32)
    target_sources(MediaFlyouts PRIVATE
        Model/WinMediaProvider.h             Model/WinMediaProvider.cpp
        Controllers/VolumeController.h       Controllers/VolumeController.cpp
        Controllers/NativeFlyoutSuppressor.h Controllers/NativeFlyoutSuppressor.cpp
        Controllers/FullscreenDetector.h     Controllers/FullscreenDetector.cpp
        Controllers/LockKeysController.h     Controllers/LockKeysController.cpp
        Controllers/MediaKeyController.h     Controllers/MediaKeyController.cpp
        Controllers/StartupController.h      Controllers/StartupController.cpp
        Controllers/TaskbarInfo.h            Controllers/TaskbarInfo.cpp
    )
    target_compile_definitions(MediaFlyouts PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    target_link_libraries(MediaFlyouts PRIVATE WindowsApp.lib)
endif()

if(MEDIAFLYOUTS_BUILD_TESTS)
    add_subdirectory(tests)
endif()
