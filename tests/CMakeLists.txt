# --- Cross-platform app classes the tests exercise (compiled into a helper lib) ---
qt_add_library(mediaflyout_testlib STATIC
    ${CMAKE_SOURCE_DIR}/Model/Settings.h         ${CMAKE_SOURCE_DIR}/Model/Settings.cpp
    ${CMAKE_SOURCE_DIR}/Model/AlbumArtProvider.h ${CMAKE_SOURCE_DIR}/Model/AlbumArtProvider.cpp
    ${CMAKE_SOURCE_DIR}/Controllers/NowPlaying.h ${CMAKE_SOURCE_DIR}/Controllers/NowPlaying.cpp
)
target_link_libraries(mediaflyout_testlib PUBLIC mediaflyout_core Qt6::Gui Qt6::Quick)
target_include_directories(mediaflyout_testlib PUBLIC
    ${CMAKE_SOURCE_DIR}/Model
    ${CMAKE_SOURCE_DIR}/Controllers
)

# --- GoogleTest / GMock: fetched, used for pure-logic and mocking ---
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
    GIT_SHALLOW    TRUE
)
# match Qt's dynamically-linked CRT on MSVC
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# One GoogleTest executable aggregating every per-class test file.
add_executable(mediaflyout_gtest
    gtest_main.cpp
    Model/FlyoutPositionerTest.cpp
    Model/TrackInfoTest.cpp
    Model/PlaybackControllerTest.cpp
    Model/SettingsTest.cpp
    Model/AlbumArtProviderTest.cpp
    Controllers/NowPlayingTest.cpp
)
target_link_libraries(mediaflyout_gtest PRIVATE
    mediaflyout_testlib
    Qt6::Test
    GTest::gtest
    GTest::gmock
)
add_test(NAME gtest_all COMMAND mediaflyout_gtest)

# --- Qt Test: one executable per class (each file has its own QTEST_GUILESS_MAIN) ---
set(qt_test_sources
    Model/FlyoutPositionerQtTest.cpp
    Model/TrackInfoQtTest.cpp
    Model/PlaybackControllerQtTest.cpp
    Model/SettingsQtTest.cpp
    Model/AlbumArtProviderQtTest.cpp
    Controllers/NowPlayingQtTest.cpp
)
set(qt_test_names "")
foreach(src IN LISTS qt_test_sources)
    get_filename_component(name ${src} NAME_WE)
    qt_add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE Qt6::Test mediaflyout_testlib)
    add_test(NAME ${name} COMMAND ${name})
    list(APPEND qt_test_names ${name})
endforeach()

# Qt's DLLs live in its bin dir; prepend it to PATH so the test exes can load Qt at runtime.
set_tests_properties(gtest_all ${qt_test_names} PROPERTIES
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")

