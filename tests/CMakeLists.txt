# --- Qt Test: exercises core logic through the Qt test harness ---
qt_add_executable(tst_core_qt tst_core_qt.cpp)
target_link_libraries(tst_core_qt PRIVATE Qt6::Test mediaflyout_core)
add_test(NAME qt_core COMMAND tst_core_qt)

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

add_executable(test_core_gtest test_core_gtest.cpp)
target_link_libraries(test_core_gtest PRIVATE mediaflyout_core GTest::gtest_main GTest::gmock)
add_test(NAME gtest_core COMMAND test_core_gtest)

# Qt's DLLs live in its bin dir; prepend it to PATH so the test exes can load Qt6Core/Qt6Test at runtime.
set_tests_properties(qt_core gtest_core PROPERTIES
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")

