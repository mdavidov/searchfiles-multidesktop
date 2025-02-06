#pragma once

#include <QDir>
#include <QString>
#include <QStringList>

namespace Devonline
{
struct ScanParams
{
    QString origDirPath;
    QString fileNameFilter;
    QStringList fileNameSubfilters;
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
