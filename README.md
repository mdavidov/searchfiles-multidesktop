# Folder Search Cross-platform GUI app

Multi-platform desktop GUI app (C++ Qt 6) with the following features:

1. Traditional file/folder search within a folder and its sub-folders
1. Controlled maximum sub-folder depth
1. Search by file/folder name (use wildcards)
1. Search for files containing given words
1. Match case of search words (or not)
1. Exclude files containing given words
1. Exclude hidden files/folders
1. Exclude by (part of) file/folder name (do not use wildcards)

## Supported platforms (in alphabetical order)

* Linux
* Mac
* Windows

## Install

TODO

### Uninstall

This app can be uninstalled by running the __maintenancetool__ app which is located in the same folder where foldersearch app is installed.

## Build from source

### Prerequisites

Ensure the following dependencies are installed:

* Qt 6
* CMake (>= 3.20)
* Ninja

C++ compiler:

* Linux: GCC 14.2 or Clang
* macOS: Xcode (Clang) or GCC 14.2
* Windows: MSVC or MinGW

### Get source

```bash
git clone https://github.com/mdavidov/searchfiles-multidesktop
cd searchfiles-multidesktop
```

### Build

NOTE: On Windows you need to open "Developer PowerShell for VS 2022" or "Developer Command Prompt for VS 2022". Visual Studio Community Edition is free for non-comm ercial use.

While still in the root directory (searchfiles-multidesktop) do this:

```bash
cmake -S . -B build -G "Ninja Multi-Config"
cmake --build build --config Debug
# or:
cmake --build build --config Release
```
