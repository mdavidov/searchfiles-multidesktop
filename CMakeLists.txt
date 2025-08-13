cmake_minimum_required(VERSION 3.20)
project(foldersearch VERSION 8.5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED CMAKE_PREFIX_PATH)
    message(WARNING "\nWARNING: CMAKE_PREFIX_PATH must be defined "
            "as path to your Qt installation, "
            "e.g. 'cmake .. -DCMAKE_PREFIX_PATH=C:/Qt/6.9.1/msvc2022_64 ...' "
            "(or /opt/Qt/6.9.1/macos). Using DEFAULTS for Qt 6.9.1.")
    if(APPLE)
        set(CMAKE_PREFIX_PATH "/opt/Qt/6.9.1/macos")
        set(CMAKE_MAKE_PROGRAM /opt/homebrew/bin/ninja)  # this isfor Qt Creator
    elseif(UNIX)
        set(CMAKE_MAKE_PROGRAM "/opt/Qt/6.8.1/gcc_arm64")
    elseif(WIN32)
        set(CMAKE_PREFIX_PATH "C:/Qt/6.8.1/msvc2022_64")
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
)

set(PROJECT_SOURCES
    src/main.cpp
    src/aboutdialog.h
    src/aboutdialog.cpp
    src/helpdialog.h
    src/helpdialog.cpp
    src/common.h
    src/config.h
    src/config.cpp
    src/get_readable_thread_id.hpp
    src/set_thread_name.cpp
    src/set_thread_name.hpp
    src/set_thread_name_win.h
    src/fileremover-v2.hpp
    src/fileremover-v3.hpp
    src/folderscanner.hpp
    src/folderscanner.cpp
    src/foldersearch.hpp
    src/foldersearch.cpp
    src/scanparams.hpp
    src/util.h
    src/util.cpp
    src/version.h
    src/windows_symlink.hpp
    src/windows_symlink.cpp
    foldersearch.qrc
)

if (APPLE)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} src/macutils.mm)
elseif(WIN32)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} foldersearch.rc)
endif()

qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Doxygen: generate docs during build!!!
# TODO: doxygen_add_docs(docs src/)

if(APPLE)
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23 -fexperimental-library -fexceptions -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wcast-qual -Wformat=2 -Wunused" CACHE STRING "CMAKE_CXX_FLAGS" FORCE)
    set(GUI_TYPE MACOSX_BUNDLE)
    set_source_files_properties(src/macutils.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
    set_source_files_properties(${CMAKE_SOURCE_DIR}/images/foldersearch.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/images/foldersearch.icns)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER au.devonline.${PROJECT_NAME}
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/images/Info.plist
    )
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23 -fexceptions -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wcast-qual -Wformat=2 -Wunused -Wdouble-promotion -Wundef -Wcast-align" CACHE STRING "CMAKE_CXX_FLAGS" FORCE)
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
    set(CMAKE_INSTALL_PREFIX /usr/local)
elseif(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /EHsc /W4 /analyze /permissive- /wd28020")
    set(GUI_TYPE WIN32)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} foldersearch.rc)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
