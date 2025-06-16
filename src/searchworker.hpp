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

#include <QObject>
#include <QThread>
#include <QString>
#include <QDir>
#include <QFileInfo>

class SearchWorker : public QObject {
    Q_OBJECT
public:
    explicit SearchWorker(QObject* parent = nullptr);

signals:
    void resultFound(const QString& file);
    void searchCompleted();
    void progressUpdate(int value);

public slots:
    void doSearch() {
        // Perform search operations here
        // Emit signals for results and progress
        emit resultFound(filename);
        emit progressUpdate(progress);
        emit searchCompleted();
    }

    void reportProgress() {
        if (isTimeToReport()) {  // Check if enough time has passed since last update
            emit progressUpdate(currentProgress);
        }
    }

    bool isTimeToReport() {
        // Update every 100ms or so
        static QElapsedTimer timer;
        if (timer.elapsed() >= 100) {
            timer.restart();
            return true;
        }
        return false;
    }
private:
    std::atomic<bool> m_stopped{false};

public slots:
    void stop() {
        m_stopped = true;
    }

    void doSearch() {
        m_stopped = false;
        while (hasMoreToSearch && !m_stopped) {
            // Do search work
            if (m_stopped) {
                emit searchInterrupted();
                return;
            }
            // Continue search
        }
    }
};

class MainWindow : public QMainWindow {
private slots:
    void handleResult(const QString& file, const QFileInfo& finfo) {
        // Update UI with found file
        filesTable->insertRow(filesTable->rowCount());
        // Add file details to table
    }

    void progressUpdate(int value) {
        // Update progress bar or status
        progressBar->setValue(value);
        statusLabel->setText(tr("Searched %1 files...").arg(value));
    }
};

// In main window:
void MainWindow::startSearch()
{
    auto qthread = new QThread;
    auto worker = new SearchWorker;
    worker->moveToThread(qthread);

    connect(qthread, &QThread::started, worker, &SearchWorker::doSearch);
    connect(worker, &SearchWorker::searchCompleted, qthread, &QThread::quit);
    connect(worker, &SearchWorker::searchCompleted, worker, &QObject::deleteLater);
    connect(qthread, &QThread::finished, qthread, &QObject::deleteLater);

    // Connect progress and results
    connect(worker, &SearchWorker::resultFound, this, &MainWindow::handleResult);
    connect(worker, &SearchWorker::progressUpdate, this, &MainWindow::progressUpdate);

    qthread->start();
}

