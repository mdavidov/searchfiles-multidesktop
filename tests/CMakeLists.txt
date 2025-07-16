cmake_minimum_required(VERSION 3.16)

find_package(Qt6 REQUIRED COMPONENTS Test Core)

qt_standard_project_setup()

qt_add_executable(test_folderscanner
    test_folderscanner.cpp
    ../src/folderscanner.cpp
    ../src/windows_symlink.cpp
)

target_link_libraries(test_folderscanner PRIVATE
    Qt6::Test
    Qt6::Core
)

target_include_directories(test_folderscanner PRIVATE
    ../src
)

add_test(NAME test_folderscanner COMMAND test_folderscanner)
