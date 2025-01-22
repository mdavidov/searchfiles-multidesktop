cmake_minimum_required(VERSION 3.28)
project(getinfo VERSION 3.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS 
    Core
    Gui
    Widgets
)

set(PROJECT_SOURCES
    src/precompiled.h
    src/precompiled.cpp
    src/main.cpp
    src/aboutdialog.h
    src/aboutdialog.cpp
    src/helpdialog.h
    src/helpdialog.cpp
    src/common.h
    src/config.h
    src/config.cpp
    src/getinfo.h
    src/getinfo.cpp
    src/exlineedit.h
    src/exlineedit.cpp
    src/searchlineedit.h
    src/searchlineedit.cpp
    src/util.h
    src/util.cpp
    src/version.h
    resources/getinfo.qrc
    resources/icons/getinfo.png
)

qt_add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
)

# Set target properties
if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fexceptions -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wcast-qual -Wformat=2 -Wunused" CACHE STRING "CMAKE_CXX_FLAGS" FORCE)
    set(GUI_TYPE MACOSX_BUNDLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER com.example.${PROJECT_NAME}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
    include_directories(
        # /opt/homebrew/Cellar/qt/6.7.3/include
        /opt/homebrew/Cellar/qt/6.7.3/lib/QtCore.framework/Versions/A/Headers
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    link_directories(
        /opt/homebrew/Cellar/qt/6.7.3/lib
    )
elseif(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fexceptions -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wcast-qual -Wformat=2 -Wunused -Wdouble-promotion -Wundef -Wcast-align" CACHE STRING "CMAKE_CXX_FLAGS" FORCE)
    set(GUI_TYPE "")
    include_directories(
        /usr/include/x86_64-linux-gnu/qt6
        /usr/include/x86_64-linux-gnu/qt6/QtWidgets
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    link_directories(
        /usr/lib/x86_64-linux-gnu
    )
elseif(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 /EHsc /W4 /WX /analyze /permissive- /wd4514 /wd4710 /wd4820 /wd4464 /wd5045")
    set(GUI_TYPE WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    # Configure precompiled headers
    target_precompile_headers(${PROJECT_NAME} PRIVATE
        src/precompiled.h
    )
endif()
message(STATUS "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")

# Link Qt6 libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Installation configuration
# include(GNUInstallDirs)
#
# Install binary
# install(TARGETS ${PROJECT_NAME}
#     BUNDLE DESTINATION .
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
# )
#
# Platform-specific deployment
# if(WIN32)
#     # Windows deployment
#     get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
#     get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
#
#     # Deploy Qt dependencies
#     add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#         COMMAND "${_qt_bin_dir}/windeployqt.exe"
#                 --no-compiler-runtime
#                 --no-translations
#                 --no-system-d3d-compiler
#                 \"$<TARGET_FILE:${PROJECT_NAME}>\"
#         COMMENT "Deploying Qt dependencies..."
#     )
# elseif(APPLE)
#     # macOS deployment
#     add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#         COMMAND "${_qt_bin_dir}/macdeployqt"
#                 \"$<TARGET_FILE_DIR:${PROJECT_NAME}>/../..\"
#                 -always-overwrite
#         COMMENT "Deploying Qt dependencies..."
#     )
# elseif(UNIX AND NOT APPLE)
#     # Linux installation
#
#     # Install desktop file
#     configure_file(
#         ${CMAKE_CURRENT_SOURCE_DIR}/resources/${PROJECT_NAME}.desktop.in
#         ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.desktop
#         @ONLY
#     )
#     install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.desktop
#         DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
#     )
#
#     # Install application icon
#     install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/${PROJECT_NAME}.png
#         DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
#     )
#
#     # Install additional application data
#     install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/data
#         DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
#     )
#
#     # Install documentation
#     install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/README.md
#         DESTINATION ${CMAKE_INSTALL_DOCDIR}
#     )
#
#     # Deploy Qt dependencies for Linux
#     if(NOT DEFINED ENV{DESTDIR})
#         set(DESTDIR "")
#     endif()
#
#     # Install Qt plugins and libraries
#     include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/LinuxDeployQt.cmake OPTIONAL)
# endif()
