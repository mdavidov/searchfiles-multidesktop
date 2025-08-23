#pragma once
//
// Copyright (c) Milivoj (Mike) DAVIDOV
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
// EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//

#include "common.hpp"
#include "scanparams.hpp"
#include "windows_symlink.hpp"
#include <atomic>
#include <map>
#include <shared_mutex>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QObject>
#include <QString>
#include <QQueue>
#include <QTimer>
#include <QElapsedTimer>

/// @mainpage
/// # FolderSearch multi-platform GUI app
///
/// Multi-platform desktop GUI app (C++ Qt6) with the following features:
///
/// * Traditional file/folder search within a folder and its sub-folders.
///   This is useful in large, remote / networked folder structures
///   that are not indexed by the local search indexer.
/// * Controlled maximum sub-folder depth
/// * Search by file/folder name (use wildcards)
/// * Search for files containing given words
/// * Match case of search words (or not)
/// * Exclude files containing given words
/// * Exclude hidden files/folders
/// * Exclude by (part of) file/folder name (do not use wildcards)
///
/// ## Supported platforms
///
/// * Windows
/// * Mac
/// * Linux
///
/// ## Install app binaries
///
/// Visit https://github.com/mdavidov/searchfiles-multidesktop/releases/,
/// click the latest release name and select the installer for your platform.
///
/// * Windows: installer-windows-intel-x86_64.exe
/// * Mac: installer-mac-apple-arm64.app.zip
/// * Linux: TODO (can build from source in the meantime, see below)
///
/// ## Uninstall
///
/// This app can be uninstalled by running the maintenancetool.exe / maintenancetool.app.
/// It's located in the same folder where foldersearch.exe / foldersearch.app is installed;
/// by default it's $HOME/DevOnline/foldersearch.
///
/// ## Build from source
///
/// ### Prerequisites
///
/// Ensure the following dependencies are installed:
///
/// * Qt 6
/// * CMake (>= 3.20)
/// * Ninja
/// * Optionally install VS Code (highly recommended).
///
/// C++ compiler:
///
/// * Windows: MSVC is recommended (or MinGW).
/// * Linux: GCC 14 (g++-14) or Clang
/// * macOS: Xcode (Clang) or GCC 14
///
/// ### Get source
///
/// ```bash
/// git clone https://github.com/mdavidov/searchfiles-multidesktop
/// cd searchfiles-multidesktop
/// ```
///
/// ### Build
///
/// On all three supported platforms VS Code can be used, but CMAKE_PREFIX_PATH
/// must be first set in the CMakeLists.txt file (above the line that checks
/// if CMAKE_PREFIX_PATH is not defined):
///
/// ```cmake
/// set(CMAKE_PREFIX_PATH "/path/to/Qt")
/// ```
///
/// Replace /path/to/Qt with the path to your Qt installation
/// (e.g. C:/Qt/6.9.1/msvc2022_64 or /opt/Qt/6.9.1/macos)
///
/// * Run VS Code and open the cloned sources folder (e.g. ```code .``` or ```code searchfiles-multidesktop``` if you didn't cd into it)
/// * If the compiler-kit-selection drop-down opens, then select your preferred one
/// * VS Code bootstraps the build automatically
/// * Press F7 to build
///
/// Otherwise follow the below instructions.
///
/// #### Windows
///
/// It's best to have Visual Studio (e.g. 2022) installed.
/// Open "Developer PowerShell for VS 2022" or "Developer Command Prompt for VS 2022".
/// __NOTE__: Visual Studio Community Edition is free (for non-commercial use).
///
/// Assuming you changed dir to searchfiles-multidesktop do this:
///
/// ```bash
/// mkdir build && cd build
/// cmake .. -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH=/path/to/Qt
/// cmake --build . --config Debug  # or Release
/// ```
///
/// Replace /path/to/Qt with the path to your Qt installation
/// (e.g. C:/Qt/6.9.1/msvc2022_64 or /opt/Qt/6.9.1/macos)
///
/// On Windows instead of building from the command line,
/// you can use Visual Studio to open the CMake generated
/// solution file (foldersearch.sln). Then build the app from
/// menu BUILD -> Build Solution or press F7.
///
/// #### Mac and Linux
///
/// Assuming you changed directory to searchfiles-multidesktop do this:
///
/// ```bash
/// mkdir build && cd build
/// cmake .. -G "Ninja Multi-Config" -DCMAKE_PREFIX_PATH=/path/to/Qt
/// ninja
/// # or
/// cmake --build . --config Debug  # or Release
/// ```
///
/// Replace /path/to/Qt with the path to your Qt installation
/// (e.g. C:/Qt/6.9.1/msvc2022_64 or /opt/Qt/6.9.1/macos)

using uint64pair = std::pair<uint64_t, uint64_t>;
class TestFolderScanner;

namespace mmd
{
bool isSymbolic(const QFileInfo& info);


/// @brief FolderScanner class scans a folder and its sub-folders
/// for files and folders, and emits signals for each found item.
/// It can also remove files and folders, and report progress.
/// It is used by the MainWindow class to perform folder scanning and file removal.
/// It is a QObject, so it can be used with signals and slots.
/// It is not thread-safe, so it should be used in a single thread.
/// It can be used with a QThread to perform scanning and removal in a separate thread.
/// It can also be used with a jthread to perform scanning and removal in a separate thread
/// @author Milivoj (Mike) DAVIDOV
///
class FolderScanner : public QObject {
    Q_OBJECT
public:
    explicit FolderScanner(QObject* parent=nullptr);
    bool isStopped() const;
    ScanParams params{};
    quint64 combinedSize(const QFileInfoList& items);

signals:
    void itemFound(const QString& path, const QFileInfo& info);
    void itemSized(const QString& path, const QFileInfo& info);
    void itemRemoved(int row, quint64 count, quint64 size, quint64 nbrDeleted);
    void progressUpdate(const QString& path, quint64 totCount, quint64 totSize);
    void scanComplete();
    void scanCancelled();
    void removalComplete(bool success);
    void removalCancelled();

public slots:
    void stop();
    void deepScan(const QString& startPath, const int maxDepth);
    uint64pair deepCountSize(const QString& startPath);
    void deepRemove(const IntQStringMap& itemList);
    void deepRemoveLimited(const IntQStringMap& itemList, const int maxDepth);
    bool deepRemLimitedImpl(const QString& startPath, const int maxDepth, quint64& nbrDeleted);
    bool doRemoveOneFileOrDir(const QFileInfo& info, int row, quint64& nbrDeleted);
    bool rmEmptyDir(const QString& dirPath, int row, quint64& nbrDeleted);

public:
    void zeroCounters();
    bool appendOrExcludeItem(const QString& dirPath, const QFileInfo& info);
    void getAllDirs(const QString& path, QFileInfoList& infos);
    void getFileInfos(const QString& path, QFileInfoList& infos) /*const*/;
    void getAllItems(const QString& path, QFileInfoList& infos) const;
    // Not necessary: void updateTotals(const QString& path);
    bool stringContainsAllWords(const QString& str, const QStringList& words);
    bool stringContainsAnyWord(const QString& str, const QStringList& words);
    bool fileContainsAllWordsChunked(const QString& path, const QStringList& words);
    bool fileContainsAnyWordChunked(const QString& path, const QStringList& words);

private:
    mutable std::shared_mutex mutex;  // mutable allows modification in const methods
    bool stopped{ false };

    qint64 prevEvents{ 0 };
    QElapsedTimer eventsTimer;
    void processEvents();

    qint64 prevProgress{ 0 };
    QElapsedTimer progressTimer;
    void reportProgress(const QString& path, bool doit = false);

    quint64 dirCount{0};
    quint64 foundCount{0};
    quint64 foundSize{0};
    quint64 symlinkCount;
    quint64 totCount{0};
    quint64 totSize{0};
};

}

Q_DECLARE_METATYPE(mmd::FolderScanner)
