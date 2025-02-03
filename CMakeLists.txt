cmake_minimum_required(VERSION 3.28)
project(foldersearch VERSION 3.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

IF(APPLE)
    SET(Qt6_DIR "/opt/Qt/6.8.1/macos/lib/cmake/Qt6")
ELSEIF(UNIX)
ELSEIF(WIN32)
    SET(Qt6_DIR "C:/Qt/6.8.1/msvc2022_64/lib/cmake/Qt6")
ENDIF()

FIND_PACKAGE(Qt6 REQUIRED COMPONENTS
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
    src/foldersearch.hpp
    src/foldersearch.cpp
    src/exlineedit.h
    src/exlineedit.cpp
    src/keypress.hpp
    src/keypress.cpp
    src/searchlineedit.h
    src/searchlineedit.cpp
    src/util.h
    src/util.cpp
    src/version.h
    foldersearch.qrc
)

if (APPLE)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} src/macutils.mm)
elseif (UNIX)
elseif(WIN32)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} foldersearch.rc)
endif()
MESSAGE(STATUS "PROJECT_SOURCES: ${PROJECT_SOURCES}")

qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
qt_add_executable(qt-file-readall src/qt-file-readall.cpp)

# Link Qt6 libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

# Set target properties
if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fexceptions -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wcast-qual -Wformat=2 -Wunused" CACHE STRING "CMAKE_CXX_FLAGS" FORCE)
    set(GUI_TYPE MACOSX_BUNDLE)
    set_source_files_properties(src/macutils.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
    set_source_files_properties(${CMAKE_SOURCE_DIR}/images/foldersearch.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/images/foldersearch.icns)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER au.devonline.${PROJECT_NAME}
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/images/Info.plist
    )
    include_directories(
        /opt/Qt/6.8.1/macos/include
        /opt/Qt/6.8.1/macos/lib/QtCore.framework/Headers
        /opt/Qt/6.8.1/macos/lib/QtWidgets.framework/Headers
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/images
    )
    link_directories(
        /opt/Qt/6.8.1/macos/lib
    )
elseif(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fexceptions -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wcast-qual -Wformat=2 -Wunused -Wdouble-promotion -Wundef -Wcast-align" CACHE STRING "CMAKE_CXX_FLAGS" FORCE)
    set(GUI_TYPE "")
    # Install desktop file
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/foldersearch.desktop.in
                  ${CMAKE_CURRENT_BINARY_DIR}/foldersearch.desktop)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/foldersearch.desktop
            DESTINATION share/applications)
    # Install icon
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/images/foldersearch.png
            DESTINATION share/icons/hicolor/32x32/apps
            RENAME ${PROJECT_NAME}.png)
    SET(CMAKE_INSTALL_PREFIX /usr/local)
    include_directories(
        /usr/include/x86_64-linux-gnu/qt6
        /usr/include/x86_64-linux-gnu/qt6/QtCore
        /usr/include/x86_64-linux-gnu/qt6/QtWidgets
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/images
    )
    link_directories(
        /usr/lib/x86_64-linux-gnu
    )
elseif(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 /EHsc /W4 /WX /analyze /permissive- /wd4514 /wd4710 /wd4820 /wd4464 /wd5045")
    set(GUI_TYPE WIN32)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} foldersearch.rc)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    # Configure precompiled headers
    target_precompile_headers(${PROJECT_NAME} PRIVATE
        src/precompiled.h
    )
    include_directories(
        C:/Qt/6.8.1/msvc2022_64/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/images
    )
    link_directories(
        C:/Qt/6.8.1/msvc2022_64/lib
    )
endif()
message(STATUS "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")

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
