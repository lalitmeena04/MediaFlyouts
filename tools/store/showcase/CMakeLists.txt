# Store screenshot renderer. Dev tool only: it stays in the build tree,
# because everything in <root>/bin ships.
qt_add_executable(MediaFlyoutsShowcase
    main.cpp
    Captions.cpp
    Captions.h
    DemoArt.cpp
    DemoArt.h
    ShowcaseDriver.cpp
    ShowcaseDriver.h
    ${CMAKE_SOURCE_DIR}/Controllers/AppTranslator.cpp
    ${CMAKE_SOURCE_DIR}/Controllers/AppTranslator.h
    ${CMAKE_SOURCE_DIR}/Controllers/BrowserArtworkImage.cpp
    ${CMAKE_SOURCE_DIR}/Controllers/BrowserArtworkImage.h
    ${CMAKE_SOURCE_DIR}/Controllers/NowPlaying.cpp
    ${CMAKE_SOURCE_DIR}/Controllers/NowPlaying.h
    ${CMAKE_SOURCE_DIR}/Controllers/ThemeController.cpp
    ${CMAKE_SOURCE_DIR}/Controllers/ThemeController.h
    ${CMAKE_SOURCE_DIR}/Model/AlbumArtProvider.cpp
    ${CMAKE_SOURCE_DIR}/Model/AlbumArtProvider.h
    ${CMAKE_SOURCE_DIR}/Model/Settings.cpp
    ${CMAKE_SOURCE_DIR}/Model/Settings.h
)
target_include_directories(MediaFlyoutsShowcase PRIVATE
    ${CMAKE_SOURCE_DIR}/Controllers
    ${CMAKE_SOURCE_DIR}/Model
)
target_compile_definitions(MediaFlyoutsShowcase PRIVATE
    MEDIAFLYOUTS_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)
target_link_libraries(MediaFlyoutsShowcase PRIVATE
    mediaflyout_core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Widgets
)

# The views load their icon fonts from this resource path.
qt_add_resources(MediaFlyoutsShowcase "showcase_fonts"
    PREFIX "/qt/qml/MediaFlyouts"
    BASE "${CMAKE_SOURCE_DIR}/Views"
    FILES
        ${CMAKE_SOURCE_DIR}/Views/assets/fonts/FluentSystemIcons-Filled.ttf
        ${CMAKE_SOURCE_DIR}/Views/assets/fonts/FontAwesome6-Solid.ttf
)

if(MEDIAFLYOUTS_BUILD_TESTS)
    # Renders every part and poster into a scratch folder. Fails on a QML
    # warning, so a view that outgrows the demo data is caught here. The
    # caption file is checked for every shipped language; rendering the source
    # language and one translated, right-to-left language keeps the test short.
    get_target_property(_qmake_path Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_path}" DIRECTORY)
    add_test(NAME ShowcaseSmokeTest COMMAND MediaFlyoutsShowcase --check --langs en,ar)
    set_tests_properties(ShowcaseSmokeTest PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${_qt_bin_dir}"
        TIMEOUT 300
    )
endif()
