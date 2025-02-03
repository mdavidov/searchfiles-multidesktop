#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QString>
#include <QTextStream>
#include <QDebug>

QString readFileContents(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return QString();
    
    return QString::fromUtf8(file.readAll());
}

// QStringView readFileContentsView(const QStringView& filePath)
// {
//     QFile file(filePath);
//     if (!file.open(QIODevice::ReadOnly))
//         return QString();
//    
//     return QString::fromUtf8(file.readAll());
// }

QByteArray readBinaryFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return QByteArray();
    
    return file.readAll();
}

QString readFileContentsText(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODeviceBase::ReadOnly))
        return QString();
    
    QTextStream in(&file);
    return in.readAll();
}

QByteArray readLargeFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return QByteArray();
    
    QByteArray content;
    constexpr qint64 chunkSize = 1024 * 1024; // 1MB chunks
    while (!file.atEnd()) {
        content.append(file.read(chunkSize));
    }
    return content;
}

QString readFileWithEncoding(const QString& filePath, const QStringConverter::Encoding encoding)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return QString();
    
    QTextStream in(&file);
    in.setEncoding(encoding);  // e.g. QStringConverter::Utf8
    return in.readAll();
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        qWarning("Usage: %s <file_path>", argv[0]);
        return 1;
    }

    auto fileName = QString(argv[1]);
    auto filePath = QFileInfo(fileName).absoluteFilePath();
    const auto content = readFileContents(filePath);
    if (content.isEmpty()) {
        qWarning("Failed to read file: %s", qPrintable(fileName));
        return 1;
    }
    qInfo() << "\n" << filePath << "\n\n" << content << "\n\n";

    // Process the file content here
    const auto text = QString("qt-file-readall.cpp");
    if (content.contains(text))
        qInfo() << "File contains " << text;
    else
        qInfo() << "File does NOT contain " << text;

    return 0;
}
