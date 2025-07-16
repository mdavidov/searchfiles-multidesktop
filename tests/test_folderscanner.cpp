#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTextStream>
#include "../src/folderscanner.hpp"

using namespace Devonline;

class TestFolderScanner : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testStringContainsAllWords();
    void testStringContainsAnyWord();
    void testFileContainsWords();
    void testStopFunctionality();
    void testBasicScan();
    void testDepthLimitedScan();
    void testExclusionPatterns();
    void testCaseSensitivity();

private:
    void createTestFile(const QString& path, const QString& content);
    void createTestDir(const QString& path);
    
    QTemporaryDir* tempDir;
    FolderScanner* scanner;
    QString testDirPath;
};

void TestFolderScanner::initTestCase()
{
    tempDir = new QTemporaryDir();
    QVERIFY(tempDir->isValid());
    testDirPath = tempDir->path();
}

void TestFolderScanner::cleanupTestCase()
{
    delete tempDir;
}

void TestFolderScanner::init()
{
    scanner = new FolderScanner();
}

void TestFolderScanner::cleanup()
{
    delete scanner;
}

void TestFolderScanner::createTestFile(const QString& path, const QString& content)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QTextStream stream(&file);
    stream << content;
}

void TestFolderScanner::createTestDir(const QString& path)
{
    QDir dir;
    QVERIFY(dir.mkpath(path));
}

void TestFolderScanner::testStringContainsAllWords()
{
    scanner->params.matchCase = false;
    
    QStringList words{"hello", "world"};
    // Test through public interface - create test files
    QString testFile1 = testDirPath + "/hello_world.txt";
    QString testFile2 = testDirPath + "/hello_only.txt";
    createTestFile(testFile1, "Hello World Test");
    createTestFile(testFile2, "Hello Test");
    
    QStringList words2{"hello", "world"};
    QVERIFY(scanner->fileContainsAllWordsChunked(testFile1, words2));
    QVERIFY(!scanner->fileContainsAllWordsChunked(testFile2, words2));
}

void TestFolderScanner::testStringContainsAnyWord()
{
    scanner->params.matchCase = false;
    
    QString testFile1 = testDirPath + "/hello_test.txt";
    QString testFile2 = testDirPath + "/world_test.txt";
    QString testFile3 = testDirPath + "/only_test.txt";
    createTestFile(testFile1, "Hello Test");
    createTestFile(testFile2, "Test World");
    createTestFile(testFile3, "Test Only");
    
    QStringList words{"hello", "world"};
    QVERIFY(scanner->fileContainsAnyWordChunked(testFile1, words));
    QVERIFY(scanner->fileContainsAnyWordChunked(testFile2, words));
    QVERIFY(!scanner->fileContainsAnyWordChunked(testFile3, words));
}

void TestFolderScanner::testFileContainsWords()
{
    QString testFile = testDirPath + "/test.txt";
    createTestFile(testFile, "This is a test file with hello world content");
    
    scanner->params.matchCase = false;
    QStringList searchWords{"hello", "world"};
    QStringList exclusionWords{"exclude", "skip"};
    
    QVERIFY(scanner->fileContainsAllWordsChunked(testFile, searchWords));
    QVERIFY(!scanner->fileContainsAnyWordChunked(testFile, exclusionWords));
}

void TestFolderScanner::testStopFunctionality()
{
    QVERIFY(!scanner->isStopped());
    scanner->stop();
    QVERIFY(scanner->isStopped());
}

void TestFolderScanner::testBasicScan()
{
    QString subDir = testDirPath + "/subdir";
    createTestDir(subDir);
    createTestFile(testDirPath + "/file1.txt", "content1");
    createTestFile(subDir + "/file2.txt", "content2");
    
    scanner->params.inclFiles = true;
    scanner->params.inclFolders = true;
    scanner->params.exclHidden = true;
    
    QSignalSpy itemFoundSpy(scanner, &FolderScanner::itemFound);
    QSignalSpy scanCompleteSpy(scanner, &FolderScanner::scanComplete);
    
    scanner->deepScan(testDirPath, -1);
    
    QVERIFY(scanCompleteSpy.wait(5000));
    QVERIFY(itemFoundSpy.count() >= 2);
}

void TestFolderScanner::testDepthLimitedScan()
{
    QString level1 = testDirPath + "/level1";
    QString level2 = level1 + "/level2";
    createTestDir(level2);
    createTestFile(level1 + "/file1.txt", "content");
    createTestFile(level2 + "/file2.txt", "content");
    
    scanner->params.inclFiles = true;
    scanner->params.inclFolders = false;
    
    QSignalSpy itemFoundSpy(scanner, &FolderScanner::itemFound);
    QSignalSpy scanCompleteSpy(scanner, &FolderScanner::scanComplete);
    
    scanner->deepScan(testDirPath, 1);
    
    QVERIFY(scanCompleteSpy.wait(5000));
    
    bool foundFile1 = false;
    bool foundFile2 = false;
    
    for (int i = 0; i < itemFoundSpy.count(); ++i) {
        QString path = itemFoundSpy.at(i).at(0).toString();
        if (path.contains("file1.txt")) foundFile1 = true;
        if (path.contains("file2.txt")) foundFile2 = true;
    }
    
    QVERIFY(foundFile1);
    QVERIFY(!foundFile2);
}

void TestFolderScanner::testExclusionPatterns()
{
    createTestFile(testDirPath + "/include.txt", "content");
    createTestFile(testDirPath + "/exclude.txt", "content");
    
    scanner->params.inclFiles = true;
    scanner->params.exclFilePatterns = QStringList{"exclude"};
    
    QSignalSpy itemFoundSpy(scanner, &FolderScanner::itemFound);
    QSignalSpy scanCompleteSpy(scanner, &FolderScanner::scanComplete);
    
    scanner->deepScan(testDirPath, -1);
    
    QVERIFY(scanCompleteSpy.wait(5000));
    
    bool foundInclude = false;
    bool foundExclude = false;
    
    for (int i = 0; i < itemFoundSpy.count(); ++i) {
        QString path = itemFoundSpy.at(i).at(0).toString();
        if (path.contains("include.txt")) foundInclude = true;
        if (path.contains("exclude.txt")) foundExclude = true;
    }
    
    QVERIFY(foundInclude);
    QVERIFY(!foundExclude);
}

void TestFolderScanner::testCaseSensitivity()
{
    createTestFile(testDirPath + "/Test.txt", "Hello World");
    
    scanner->params.inclFiles = true;
    scanner->params.searchWords = QStringList{"hello"};
    
    QSignalSpy itemFoundSpy(scanner, &FolderScanner::itemFound);
    QSignalSpy scanCompleteSpy(scanner, &FolderScanner::scanComplete);
    
    scanner->params.matchCase = false;
    scanner->deepScan(testDirPath, -1);
    QVERIFY(scanCompleteSpy.wait(5000));
    QVERIFY(itemFoundSpy.count() > 0);
    
    itemFoundSpy.clear();
    scanCompleteSpy.clear();
    
    scanner->params.matchCase = true;
    scanner->deepScan(testDirPath, -1);
    QVERIFY(scanCompleteSpy.wait(5000));
    QVERIFY(itemFoundSpy.count() == 0);
}

QTEST_MAIN(TestFolderScanner)
#include "test_folderscanner.moc"