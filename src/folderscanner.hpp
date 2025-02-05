#include <QObject>
#include <QDir>
#include <QFileInfo>
#include <QQueue>
#include <QTimer>

class FolderScanner : public QObject {
    Q_OBJECT
public:
    explicit FolderScanner(QObject* parent = nullptr) : QObject(parent) {
        connect(&progressTimer, &QTimer::timeout, this, &FolderScanner::reportProgress);
        progressTimer.setInterval(100); // Report progress every 100ms
    }

signals:
    void fileFound(const QString& path, const QFileInfo& info);
    void folderFound(const QString& path, const QFileInfo& info);
    void progressUpdate(int itemsProcessed);
    void scanComplete();
    void scanCancelled();

public slots:
    void startScan(const QString& startPath) {
        stopped = false;
        itemCount = 0;
        progressTimer.start();

        QQueue<QString> folderQueue;
        folderQueue.enqueue(startPath);

        while (!folderQueue.isEmpty() && !stopped) {
            QString currentPath = folderQueue.dequeue();
            QDir currentDir(currentPath);

            QFileInfoList entries = currentDir.entryInfoList(
                QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);

            for (const QFileInfo& entry : entries) {
                if (stopped) {
                    progressTimer.stop();
                    emit scanCancelled();
                    return;
                }
                itemCount++;
                // Do not follow symlinks
                if (entry.isDir() && !entry.isSymLink()) {
                    folderQueue.enqueue(entry.absoluteFilePath());
                    emit folderFound(entry.absoluteFilePath(), entry);
                }
                else {
                    emit fileFound(entry.absoluteFilePath(), entry);
                }
            }
        }
        progressTimer.stop();
        emit scanComplete();
    }

    void stop() {
        stopped = true;
    }

private slots:
    void reportProgress() {
        emit progressUpdate(itemCount);
    }

private:
    QTimer progressTimer;
    std::atomic<bool> stopped{false};
    std::atomic<int> itemCount{0};
};

// Usage example:
void MainWindow::scanFolder() {
    QString startPath = QFileDialog::getExistingDirectory(this, 
        "Select Folder to Scan", QDir::homePath());
    if (startPath.isEmpty()) return;

    // Create scanner and move to thread
    QThread* scanThread = new QThread(this);
    FolderScanner* scanner = new FolderScanner;
    scanner->moveToThread(scanThread);

    // Connect signals/slots
    connect(scanThread, &QThread::started, 
            [scanner, startPath]() { scanner->startScan(startPath); });

    connect(scanner, &FolderScanner::fileFound, this, &MainWindow::handleFileFound);
    connect(scanner, &FolderScanner::folderFound, this, &MainWindow::handleFolderFound);
    connect(scanner, &FolderScanner::progressUpdate, this, &MainWindow::updateProgress);

    connect(scanner, &FolderScanner::scanComplete, scanThread, &QThread::quit);
    connect(scanner, &FolderScanner::scanCancelled, scanThread, &QThread::quit);
    connect(scanThread, &QThread::finished, scanner, &QObject::deleteLater);
    connect(scanThread, &QThread::finished, scanThread, &QObject::deleteLater);

    // Connect cancel button
    connect(cancelButton, &QPushButton::clicked, scanner, &FolderScanner::stop);

    // Start scanning
    scanThread->start();
}

void MainWindow::handleFileFound(const QString& path, const QFileInfo& info) {
    // Add to table widget or model
    const auto row = filesTable->rowCount();
    filesTable->insertRow(row);
    filesTable->setItem(row, 0, new QTableWidgetItem(info.fileName()));
    filesTable->setItem(row, 1, new QTableWidgetItem(path));
    filesTable->setItem(row, 2, new QTableWidgetItem(QString::number(info.size())));
}

void MainWindow::updateProgress(int count) {
    statusLabel->setText(tr("Processed %1 items...").arg(count));
}
