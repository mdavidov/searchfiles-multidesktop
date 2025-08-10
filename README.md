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

## Build from source

### Prerequisites

Ensure the following dependencies are installed:

* Qt 6
* CMake (>= 3.20)
* Ninja

#### C++ compiler

* Linux: GCC 14.2 or Clang
* macOS: Xcode (clang)
* Windows: MSVC or MinGW

## Uninstallation

This app can be uninstalled by running the __maintenancetool__ app which is located in the same folder where foldersearch app is installed.
