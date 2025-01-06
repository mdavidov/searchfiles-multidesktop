cmake_minimum_required(VERSION 3.28)
project(getinfo VERSION 3.0.0 LANGUAGES CXX)

# Configure Qt settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt packages
find_package(Qt6 REQUIRED COMPONENTS 
    Core
    Gui
    Widgets
)

# Collect source files
set(PROJECT_SOURCES
    src/precompiled.h
    src/precompiled.cpp
    src/main.cpp
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
)

# Create executable
qt_add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
)

# Set target properties
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
elseif(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER com.example.${PROJECT_NAME}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
endif()

# Configure precompiled headers
target_precompile_headers(${PROJECT_NAME} PRIVATE
    src/precompiled.h
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Include directories
# target_include_directories(${PROJECT_NAME} PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/src
# )
#
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
