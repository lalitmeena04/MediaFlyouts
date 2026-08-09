qt_add_library(mediaflyout_core STATIC
    flyoutpositioner.h flyoutpositioner.cpp
    trackinfo.h trackinfo.cpp
    playbackcontroller.h playbackcontroller.cpp
)

target_link_libraries(mediaflyout_core PUBLIC Qt6::Core)
target_include_directories(mediaflyout_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
