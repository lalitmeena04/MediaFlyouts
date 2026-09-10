cmake_minimum_required(VERSION 3.21)

# Single version source for app, packaging, and CI.
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION.txt" MEDIAFLYOUTS_VERSION_BASE)
string(STRIP "${MEDIAFLYOUTS_VERSION_BASE}" MEDIAFLYOUTS_VERSION_BASE)

# CMake project version needs numeric x.y.z. Strip pre-release suffix.
string(REGEX MATCH "^[0-9]+(\\.[0-9]+)*" MEDIAFLYOUTS_VERSION_NUMERIC "${MEDIAFLYOUTS_VERSION_BASE}")

project(MediaFlyouts VERSION ${MEDIAFLYOUTS_VERSION_NUMERIC} LANGUAGES CXX)

# Release builds have plain version.
# Dev builds have "-<build number>" suffix.
# Build number can be explicit or use git count and short hash.
set(MEDIAFLYOUTS_VERSION "${MEDIAFLYOUTS_VERSION_BASE}")
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release" OR DEFINED MEDIAFLYOUTS_BUILD_NUMBER)
    if(NOT DEFINED MEDIAFLYOUTS_BUILD_NUMBER)
        find_package(Git QUIET)
        if(GIT_FOUND)
            execute_process(COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE MEDIAFLYOUTS_GIT_COUNT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
            execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE MEDIAFLYOUTS_GIT_SHA OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        endif()
        if(MEDIAFLYOUTS_GIT_COUNT AND MEDIAFLYOUTS_GIT_SHA)
            set(MEDIAFLYOUTS_BUILD_NUMBER "${MEDIAFLYOUTS_GIT_COUNT}.${MEDIAFLYOUTS_GIT_SHA}")
        elseif(MEDIAFLYOUTS_GIT_COUNT)
            set(MEDIAFLYOUTS_BUILD_NUMBER "${MEDIAFLYOUTS_GIT_COUNT}")
        else()
            set(MEDIAFLYOUTS_BUILD_NUMBER "dev")
        endif()
    endif()
    set(MEDIAFLYOUTS_VERSION "${MEDIAFLYOUTS_VERSION_BASE}-${MEDIAFLYOUTS_BUILD_NUMBER}")
endif()
message(STATUS "MediaFlyouts version: ${MEDIAFLYOUTS_VERSION}")

# Expose version to C++ through Version.h.
add_compile_definitions(MEDIAFLYOUTS_VERSION="${MEDIAFLYOUTS_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Network Qml Quick Test Widgets)
qt_standard_project_setup(REQUIRES 6.5)

# File logging uses spdlog. Prefer system package. Else fetch pinned release.
find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
    include(FetchContent)
    FetchContent_Declare(spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.14.1
    )
    set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(spdlog)
endif()

option(MEDIAFLYOUTS_BUILD_TESTS "Build the unit tests" ON)
if(MEDIAFLYOUTS_BUILD_TESTS)
    enable_testing()
endif()

# --- Model: cross-platform domain logic for app and tests ---
qt_add_library(mediaflyout_core STATIC
    Logging.cpp
    Logging.h
    Model/BatteryAlertPolicy.cpp
    Model/BatteryAlertPolicy.h
    Model/BatteryUtils.cpp
    Model/BatteryUtils.h
    Model/Capabilities.cpp
    Model/Capabilities.h
    Model/DualSenseProtocol.cpp
    Model/DualSenseProtocol.h
    Model/FastPairProtocol.cpp
    Model/FastPairProtocol.h
    Model/Fft.cpp
    Model/Fft.h
    Model/FlyoutPositioner.cpp
    Model/FlyoutPositioner.h
    Model/HidppProtocol.cpp
    Model/HidppProtocol.h
    Model/PlaybackController.cpp
    Model/PlaybackController.h
    Model/PowerAlertPolicy.cpp
    Model/PowerAlertPolicy.h
    Model/RazerProtocol.cpp
    Model/RazerProtocol.h
    Model/ReceiverProfiles.cpp
    Model/ReceiverProfiles.h
    Model/SessionSelection.cpp
    Model/SessionSelection.h
    Model/TrackInfo.cpp
    Model/TrackInfo.h
    Model/WiredPeripheralRules.cpp
    Model/WiredPeripheralRules.h
)
target_link_libraries(mediaflyout_core PUBLIC
    Qt6::Core
    spdlog::spdlog
)
# Logging on compiles TRC_DEBUG. Logging off removes TRC calls.
# Logging setup omits log file if logging is off.
option(MEDIAFLYOUTS_ENABLE_LOGGING "Enable spdlog file logging" ON)
if(MEDIAFLYOUTS_ENABLE_LOGGING)
    set(MEDIAFLYOUTS_SPDLOG_LEVEL SPDLOG_LEVEL_DEBUG)
else()
    set(MEDIAFLYOUTS_SPDLOG_LEVEL SPDLOG_LEVEL_OFF)
endif()
target_compile_definitions(mediaflyout_core PUBLIC SPDLOG_ACTIVE_LEVEL=${MEDIAFLYOUTS_SPDLOG_LEVEL})
target_include_directories(mediaflyout_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Model
)

# --- Application executable ---
qt_add_executable(MediaFlyouts WIN32
    Controllers/FlyoutPlacement.cpp
    Controllers/FlyoutPlacement.h
    Controllers/NowPlaying.cpp
    Controllers/NowPlaying.h
    Controllers/QmlLogger.cpp
    Controllers/QmlLogger.h
    Controllers/SingleInstance.cpp
    Controllers/SingleInstance.h
    Controllers/ThemeController.cpp
    Controllers/ThemeController.h
    Controllers/TrayController.cpp
    Controllers/TrayController.h
    Controllers/UpdateChecker.cpp
    Controllers/UpdateChecker.h
    main.cpp
    MainControllerClass.cpp
    MainControllerClass.h
    Model/AlbumArtProvider.cpp
    Model/AlbumArtProvider.h
    Model/Settings.cpp
    Model/Settings.h
)

target_include_directories(MediaFlyouts PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/Model
)

# Emit runnable app and windeployqt runtime into <root>/bin.
set_target_properties(MediaFlyouts PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

set_source_files_properties(Views/Fluent.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(Views/Anim.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(Views/DeviceIcons.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
# Keep font at stable resource path.
set_source_files_properties(Views/assets/fonts/FluentSystemIcons-Filled.ttf
    PROPERTIES QT_RESOURCE_ALIAS assets/fonts/FluentSystemIcons-Filled.ttf)
set_source_files_properties(Views/assets/fonts/FontAwesome6-Solid.ttf
    PROPERTIES QT_RESOURCE_ALIAS assets/fonts/FontAwesome6-Solid.ttf)

qt_add_qml_module(MediaFlyouts
    URI MediaFlyouts
    VERSION 1.0
    QML_FILES
        Views/Anim.qml
        Views/App.qml
        Views/BatteryIcon.qml
        Views/BatteryRing.qml
        Views/ChargerFlyout.qml
        Views/Dashboard.qml
        Views/DeviceIcons.qml
        Views/DockableWidget.qml
        Views/Fluent.qml
        Views/FlyoutBase.qml
        Views/IconLabel.qml
        Views/InfoTooltip.qml
        Views/LockKeysFlyout.qml
        Views/MediaFlyout.qml
        Views/NowPlayingFlyout.qml
        Views/OnboardingWindow.qml
        Views/PeekArea.qml
        Views/PeripheralCard.qml
        Views/PeripheralFlyout.qml
        Views/PeripheralRow.qml
        Views/PeripheralsWidget.qml
        Views/RoundedArt.qml
        Views/ScrubArea.qml
        Views/SettingCombo.qml
        Views/SettingRow.qml
        Views/SettingSlider.qml
        Views/TaskbarVisualizer.qml
        Views/TaskbarWidget.qml
        Views/VolumeFlyout.qml
        Views/VolumeMixerWindow.qml
    RESOURCES
        Views/assets/fonts/FluentSystemIcons-Filled.ttf
        Views/assets/fonts/FontAwesome6-Solid.ttf
        Views/assets/github-mark-dark.svg
        Views/assets/github-mark-light.svg
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
        Controllers/AccentWatcher.cpp
        Controllers/AccentWatcher.h
        Controllers/AudioCapture.cpp
        Controllers/AudioCapture.h
        Controllers/BatteryProviders/BluetoothClassicBatteryProvider.cpp
        Controllers/BatteryProviders/BluetoothClassicBatteryProvider.h
        Controllers/BatteryProviders/BluetoothLeBatteryProvider.cpp
        Controllers/BatteryProviders/BluetoothLeBatteryProvider.h
        Controllers/BatteryProviders/FastPairBatteryProvider.cpp
        Controllers/BatteryProviders/FastPairBatteryProvider.h
        Controllers/BatteryProviders/GameControllerBatteryProvider.cpp
        Controllers/BatteryProviders/GameControllerBatteryProvider.h
        Controllers/BatteryProviders/HidCollections.cpp
        Controllers/BatteryProviders/HidCollections.h
        Controllers/BatteryProviders/IBatteryProvider.h
        Controllers/BatteryProviders/LogitechBatteryProvider.cpp
        Controllers/BatteryProviders/LogitechBatteryProvider.h
        Controllers/BatteryProviders/PlayStationControllerBatteryProvider.cpp
        Controllers/BatteryProviders/PlayStationControllerBatteryProvider.h
        Controllers/BatteryProviders/RazerBatteryProvider.cpp
        Controllers/BatteryProviders/RazerBatteryProvider.h
        Controllers/BatteryProviders/WiredPeripheralProvider.cpp
        Controllers/BatteryProviders/WiredPeripheralProvider.h
        Controllers/BatteryProviders/WirelessReceiverProvider.cpp
        Controllers/BatteryProviders/WirelessReceiverProvider.h
        Controllers/BatteryStatusController.cpp
        Controllers/BatteryStatusController.h
        Controllers/BluetoothConnectionWatcher.cpp
        Controllers/BluetoothConnectionWatcher.h
        Controllers/FullscreenDetector.cpp
        Controllers/FullscreenDetector.h
        Controllers/HidChangeWatcher.cpp
        Controllers/HidChangeWatcher.h
        Controllers/LockKeysController.cpp
        Controllers/LockKeysController.h
        Controllers/MediaKeyController.cpp
        Controllers/MediaKeyController.h
        Controllers/NativeFlyoutSuppressor.cpp
        Controllers/NativeFlyoutSuppressor.h
        Controllers/PeripheralAlerts.cpp
        Controllers/PeripheralAlerts.h
        Controllers/PowerStatusController.cpp
        Controllers/PowerStatusController.h
        Controllers/StartupController.cpp
        Controllers/StartupController.h
        Controllers/TaskbarInfo.cpp
        Controllers/TaskbarInfo.h
        Controllers/VolumeController.cpp
        Controllers/VolumeController.h
        Controllers/VolumeMixer.cpp
        Controllers/VolumeMixer.h
        Controllers/WindowEffects.cpp
        Controllers/WindowEffects.h
        Model/WinMediaProvider.cpp
        Model/WinMediaProvider.h
        packaging/MediaFlyouts.rc
    )
    target_compile_definitions(MediaFlyouts PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
    target_link_libraries(MediaFlyouts PRIVATE
        WindowsApp.lib
        dwmapi.lib
        shell32.lib
        gdi32.lib
        user32.lib
        cfgmgr32.lib
        Bthprops.lib
        hid.lib
        setupapi.lib
        ws2_32.lib
    )
endif()

if(MEDIAFLYOUTS_BUILD_TESTS)
    add_subdirectory(tests)
endif()
