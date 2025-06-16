#pragma once
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) Milivoj (Mike) DAVIDOV
// All rights reserved.
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
// EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//
/////////////////////////////////////////////////////////////////////////////

#include <QDir>
#include <QString>
#include <QStringList>

namespace Devonline
{
struct ScanParams
{
    QString origDirPath;
    QStringList nameFilters;
    QDir::Filters itemTypeFilter;
    bool matchCase;
    bool inclFiles;
    bool inclFolders;
    bool inclSymlinks;
    bool exclHidden;
    QStringList searchWords;
    QStringList exclusionWords;
    QStringList exclFilePatterns;
    QStringList exclFolderPatterns;
};
}
