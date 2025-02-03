#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QQueue>
#include <QString>

// Deeply list contents of the given directory
void listFolderContents(const QString& startPath, const int maxDepth) {
    // Queue pairs of (path, depth)
    QQueue<QPair<QString, int>> folderQueue;
    folderQueue.enqueue({startPath, 0});
    
    while (!folderQueue.isEmpty()) {
        auto [currentPath, currentDepth] = folderQueue.dequeue();

        // Skip if we've exceeded max depth
        if (currentDepth > maxDepth) {
            continue;
        }
        QDir currentDir(currentPath);
        static constexpr QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System;
        QFileInfoList entries = currentDir.entryInfoList(filters);

        for (const QFileInfo& entry : entries) {
            if (entry.isDir()) {
                // Only queue if we haven't reached max depth
                if (currentDepth < maxDepth) {
                    folderQueue.enqueue({entry.absoluteFilePath(), currentDepth + 1});
                }
                qDebug() << QString("Folder[%1]: ").arg(currentDepth) 
                         << entry.absoluteFilePath();
            }
            else if (entry.isSymLink() || entry.isSymbolicLink() || entry.isShortcut()) {
                qDebug() << QString("SymLink[%1]:").arg(currentDepth)
                         << entry.absoluteFilePath();
            }
            else {
                qDebug() << QString("File[%1]:   ").arg(currentDepth) 
                         << entry.absoluteFilePath();
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if  (argc < 3) {
        qDebug() << "Usage: " << argv[0] << " <folder_path> <max_depth>";
        return 1;
    }
    listFolderContents(argv[1], QString(argv[2]).toInt());
    return 0;
}
