#pragma once
#include "windows_symlink.h"

#ifdef Q_OS_WIN

namespace Devonline
{

bool _isWindowsSymlink(const QString& path) {
  DWORD attributes = GetFileAttributesW(reinterpret_cast<LPCWSTR>(path.utf16()));
  return (attributes != INVALID_FILE_ATTRIBUTES) &&
         (attributes & FILE_ATTRIBUTE_REPARSE_POINT);
}


bool _isAppExecutionAlias(const QString& path) {
  WIN32_FIND_DATAW findData;
  HANDLE hFind = FindFirstFileW(reinterpret_cast<LPCWSTR>(path.utf16()), &findData);
  if (hFind != INVALID_HANDLE_VALUE) {
      FindClose(hFind);
      return (findData.dwReserved0 == IO_REPARSE_TAG_APPEXECLINK);
  }
  return false;
}

QString _getSymlinkTarget(const QString& path)
{
    HANDLE handle = CreateFileW((LPCWSTR)path.utf16(), FILE_READ_EA,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, 
                OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        return QString();
    }
    // Buffer for reparse point data
    const DWORD bufferSize = 2 * MAXIMUM_REPARSE_DATA_BUFFER_SIZE;
    std::vector<uint8_t> buffer(bufferSize);

    // Get the reparse point data
    DWORD bytesReturned{0};
    auto success = DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT,
                NULL, 0, buffer.data(), bufferSize, &bytesReturned, NULL);
    CloseHandle(handle);
    if (!success) {
        return QString();
    }
    REPARSE_DATA_BUFFER* reparseData = (REPARSE_DATA_BUFFER*)buffer.data();
    QString target;

    if (IsReparseTagMicrosoft(reparseData->ReparseTag) &&
        reparseData->ReparseTag == IO_REPARSE_TAG_SYMLINK)
    {
        // Get the target path from symlink reparse buffer
        const USHORT nameOffset = reparseData->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(WCHAR);
        const WCHAR* targetPath = &reparseData->SymbolicLinkReparseBuffer.PathBuffer[nameOffset];
        const USHORT targetLength = reparseData->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
        target = QString::fromWCharArray(targetPath, targetLength);

        // Remove the \??\ prefix if present
        if (target.startsWith("\\??\\"))
            target = target.mid(4);
    }
    return target;
}


QString _getAppExecLinkTarget(const QString& path) {
    HANDLE fileHandle = CreateFileW(reinterpret_cast<LPCWSTR>(path.utf16()),
                GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        return QString();
    }

    static constexpr DWORD bufferSize = 2 * MAXIMUM_REPARSE_DATA_BUFFER_SIZE;
    std::vector<unsigned char> buffer(bufferSize);
    DWORD bytesReturned{ 0 };
    bool success = DeviceIoControl(fileHandle, FSCTL_GET_REPARSE_POINT,
                NULL, 0, buffer.data(), bufferSize, &bytesReturned, NULL);
    CloseHandle(fileHandle);
    if (!success) {
        return QString();
    }

  // The reparse data contains multiple strings, with the actual target being the third string.
  // Format is: PackageID, EntryPoint, ExecutablePath, AppType
  auto reparseData = reinterpret_cast<PREPARSE_DATA_BUFFER>(buffer.data());
  if (reparseData->ReparseTag == IO_REPARSE_TAG_APPEXECLINK) {
      wchar_t* stringData = reinterpret_cast<wchar_t*>(reparseData->GenericReparseBuffer.DataBuffer);

      // Skip first two strings
      for (int i = 0; i < 2; i++) {
          stringData += wcslen(stringData) + 1;
      }

      // Third string is the executable path
      return QString::fromWCharArray(stringData);
  }
  return QString();
}


// Usage example:
void checkAppExecLink(const QString& path) {
    QString target = _getAppExecLinkTarget(path);
    if (!target.isEmpty()) {
        qDebug() << "Target executable:" << target;
    }
}

}
#endif

namespace Devonline
{

bool isWindowsSymlink(const QString& path) {
#ifdef Q_OS_WIN
    return _isWindowsSymlink(path);
#else
    return false;
#endif
}

bool isAppExecutionAlias(const QString& path) {
#ifdef Q_OS_WIN
    return _isAppExecutionAlias(path);
#else
    return false;
#endif
}

QString getWindowsSymlinkTarget(const QString& path) {
#ifdef Q_OS_WIN
    if (_isWindowsSymlink(path)) {
        return _getSymlinkTarget(path);
    }
    else if (_isAppExecutionAlias(path)) {
        return _getAppExecLinkTarget(path);
    }
    return QString();
#else
    return QString();
#endif
}

}