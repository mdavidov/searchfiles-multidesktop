#pragma once

#ifdef Q_OS_WIN

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winioctl.h>
#undef ERROR
#undef min
#undef max
#include <QString>
#include <vector>

namespace Devonline
{

typedef struct _REPARSE_DATA_BUFFER {
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union {
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR  PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR  PathBuffer[1];
        } MountPointReparseBuffer;
        struct {
            UCHAR DataBuffer[1];
        } GenericReparseBuffer;
    } DUMMYUNIONNAME;
} REPARSE_DATA_BUFFER, * PREPARSE_DATA_BUFFER;

#define REPARSE_DATA_BUFFER_HEADER_SIZE FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer)

}
#endif

#include <QString>

namespace Devonline
{
    bool  isWindowsSymlink(const QString& path);
    bool  isAppExecutionAlias(const QString& path);
    QString getWindowsSymlinkTarget(const QString& path);
}
