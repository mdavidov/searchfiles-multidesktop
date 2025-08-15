# FolderSearch multi-platform GUI app

Multi-platform desktop GUI app (C++ Qt 6) with the following features:

1. Traditional file/folder search within a folder and its sub-folders
1. Controlled maximum sub-folder depth
1. Search by file/folder name (use wildcards)
1. Search for files containing given words
1. Match case of search words (or not)
1. Exclude files containing given words
1. Exclude hidden files/folders
1. Exclude by (part of) file/folder name (do not use wildcards)

## Supported platforms

* Windows
* Mac
* Linux

## Install

Visit https://github.com/mdavidov/searchfiles-multidesktop/releases/,
click the latest release name and select an installer.

* Windows: installer-windows-intel-x86_64.exe
* Apple Mac: installer-mac-apple-arm64.app.zip
* Linux: TODO (can build from source in the meantime, see below)

## Uninstall

This app can be uninstalled by running the "__maintenancetool__"
app which is located in the same folder where foldersearch executable
is installed; by default it's $HOME/DevOnline/foldersearch folder.

## Build from source

### Prerequisites

Ensure the following dependencies are installed:

* Qt 6
* CMake (>= 3.20)
* Ninja
* Optionally install VS Code (highly recommended).

C++ compiler:

* Windows: MSVC is recommended (or MinGW).
* Linux: GCC 14 (g++-14) or Clang
* macOS: Xcode (Clang) or GCC 14

### Get source

```bash
git clone https://github.com/mdavidov/searchfiles-multidesktop
cd searchfiles-multidesktop
```

### Build

On all three supported platforms VS Code can be used, but CMAKE_PREFIX_PATH
must be first set in the CMakeLists.txt file (above the line that checks
if CMAKE_PREFIX_PATH is not defined):

```cmake
set(CMAKE_PREFIX_PATH "/path/to/Qt")
```

Replace /path/to/Qt with the path to your Qt installation
(e.g. C:/Qt/6.9.1/msvc2022_64 or /opt/Qt/6.9.1/macos)

* Run VS Code and open the cloned sources folder (e.g. ```code .``` or ```code searchfiles-multidesktop``` if you didn't cd into it)
* If the compiler-kit-selection drop-down opens, then select your preferred one
* VS Code bootstraps the build automatically
* Press F7 to build

Otherwise follow the below instructions.

#### Windows

It's best to have Visual Studio (e.g. 2022) installed.
Open "Developer PowerShell for VS 2022" or "Developer Command Prompt for VS 2022".
__NOTE__: Visual Studio Community Edition is free (for non-commercial use).

Assuming you changed dir to searchfiles-multidesktop do this:

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH=/path/to/Qt
cmake --build . --config Debug  # or Release
```

Replace /path/to/Qt with the path to your Qt installation
(e.g. C:/Qt/6.9.1/msvc2022_64 or /opt/Qt/6.9.1/macos)

On Windows instead of building from the command line,
you can use Visual Studio to open the CMake generated
solution file (foldersearch.sln). Then build the app from
menu BUILD -> Build Solution or press F7.

#### Mac and Linux

Assuming you changed directory to searchfiles-multidesktop do this:

```bash
mkdir build && cd build
cmake .. -G "Ninja Multi-Config" -DCMAKE_PREFIX_PATH=/path/to/Qt
ninja
# or
cmake --build . --config Debug  # or Release
```

Replace /path/to/Qt with the path to your Qt installation
(e.g. C:/Qt/6.9.1/msvc2022_64 or /opt/Qt/6.9.1/macos)
