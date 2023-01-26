#include "processingworker.hpp"

#include <QDir>
#include <QDirIterator>
#include <QSet>

enum class FileType
{
    Image,
    Video
};

const QMap<QString, FileType> FILE_TYPES{
        {"*.jpeg", FileType::Image}, {"*.jpg", FileType::Image}, {"*.mp4", FileType::Video}};

const QStringList KNOWN_PREFIXES{"IMG_", "VID_", "SAVE_", "PANO_"};

QString startsWithPrefix(const QString &fileName)
{
    for (const auto &prefix : KNOWN_PREFIXES)
    {
        if (fileName.startsWith(prefix))
        {
            return prefix;
        }
    }
    return QString();
}

ProcessingWorker::FileName ProcessingWorker::newPrefixedFileNamePath(
        const QFileInfo &currentFile, const QDir &destDir, const QString &prefix)
{
    // name contains valid datetime
    const QString newFileName =
            currentFile.baseName().right(currentFile.baseName().size() - prefix.size());

    const QString date = newFileName.split("_").first();
    const QDate realDate = QDate::fromString(date, "yyyyMMdd");
    const QString path =
            QStringLiteral("%1/%2").arg(realDate.year()).arg(realDate.toString("yyyy-MM-dd"));

    QString fullFileName = QStringLiteral("%1/%2/%3.%4")
                                   .arg(destDir.path())
                                   .arg(path)
                                   .arg(newFileName)
                                   .arg(currentFile.suffix());
    QFileInfo existingFile(fullFileName);

    if (existingFile.exists())
    {
        if (existingFile.size() == currentFile.size())
        {
            qDebug() << "File:" << currentFile.absoluteFilePath() << "already exists";
            return {QString(), ProcessingWorker::FileName::Status::Duplicated};
        }
        else
        {
            for (int i = 1;; ++i)
            {
                fullFileName = QStringLiteral("%1/%2/%3_%4.%5")
                                       .arg(destDir.path())
                                       .arg(path)
                                       .arg(newFileName)
                                       .arg(i)
                                       .arg(currentFile.suffix());
                existingFile.setFile(fullFileName);

                if (!existingFile.exists())
                {
                    return {fullFileName, ProcessingWorker::FileName::Status::Sufixxed};
                }
                else
                {
                    if (existingFile.size() == currentFile.size())
                    {
                        return {QString(), ProcessingWorker::FileName::Status::Duplicated};
                    }
                    else
                    {
                        continue;
                    }
                }
            }
        }
    }

    return {fullFileName, ProcessingWorker::FileName::Status::Ok};
}

ProcessingWorker::ProcessingWorker(const Settings &settings) : mStopped(false), mSettings(settings)
{
}

ProcessingWorker::~ProcessingWorker()
{
    QMetaObject::invokeMethod(this, "cleanup");
    //    mThread->wait();
}

void ProcessingWorker::process()
{
    grabStatistics();

    QDir dir(mSettings.sourcePath);
    QDir destDir(mSettings.destinationPath);

    QDirIterator it(
            dir,
            mSettings.recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);

    int count = 0;
    double size = 0.;

    while (it.hasNext())
    {
        it.next();

        const QFileInfo fileInfo = it.fileInfo();
        if (!fileInfo.isFile())
            continue;

        FileName newFilenamePath = generateNewFileName(fileInfo, destDir);

        bool copied = false;
        if (!newFilenamePath.name.isEmpty())
        {
            if (destDir.mkpath(QFileInfo(newFilenamePath.name).absolutePath()))
            {
                copied = QFile::copy(fileInfo.filePath(), newFilenamePath.name);
            }
        }
        else
        {
            if (newFilenamePath.status == FileName::Status::NotAvailable)
            {
                mReport.unhandledFiles << fileInfo.absoluteFilePath();
                mReport.unhandledSize += fileInfo.size() / 1024. / 1024.;
            }
            else
            {
                ++mReport.alreadyExisting;
            }
        }

        if (copied)
        {
            mReport.copiedFiles << fileInfo.absoluteFilePath();
            mReport.copiedSize += fileInfo.size() / 1024. / 1024.;
            if (newFilenamePath.status == FileName::Status::Sufixxed)
            {
                mReport.renamed << QStringLiteral("%1 -> %2")
                                           .arg(fileInfo.fileName())
                                           .arg(newFilenamePath.name);
            }
        }
        else
        {
            if (!newFilenamePath.name.isEmpty())
            {
                mReport.uncopiedFiles << fileInfo.filePath();
                mReport.uncopiedSize += fileInfo.size() / 1024. / 1024.;
            }
        }

        size += (fileInfo.size() / 1024. / 1024.);
        ++count;

        emit progressChanged(count, size, false);

        QThread::msleep(1);
    }
    qDebug() << "CALC DONE";

    mReport.finished = true;

    emit done(mReport);
}

void ProcessingWorker::stop()
{
    QMutexLocker locker(&mMutex);
    mStopped = true;
}

void ProcessingWorker::cleanup()
{
    //    mThread->quit();
    deleteLater();
}

void ProcessingWorker::run() {}

void ProcessingWorker::grabStatistics()
{
    QDir dir = mSettings.sourcePath;
    const auto extensions = [](const QMap<QString, FileType> &files) {
        QStringList rval;
        for (const auto &file : files.keys())
        {
            rval.append(file.toUpper());
            rval.append(file.toLower());
        }
        return rval;
    }(FILE_TYPES);
    dir.setNameFilters(extensions);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    qDebug() << "EXTENSIONS:" << extensions;

    QDirIterator it(
            dir,
            mSettings.recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);

    int count = 0;
    double size = 0;

    qDebug() << "CALC STARTED";
    while (it.hasNext())
    {
        it.next();

        const QFileInfo fileInfo = it.fileInfo();
        if (!fileInfo.isFile())
            continue;

        size += fileInfo.size() / 1024. / 1024.;
        ++count;

        emit calculationProgressChanged(count, size, false);

        QThread::msleep(1);
    }
    qDebug() << "CALC DONE";

    emit calculationProgressChanged(count, size, true);
    mReport.filesCount = count;
    mReport.size = size;
}

bool ProcessingWorker::isStopped()
{
    QMutexLocker locker(&mMutex);
    return mStopped;
}

ProcessingWorker::FileName ProcessingWorker::generateNewFileName(
        const QFileInfo &file, const QDir &destDir)
{
    const QString prefix = startsWithPrefix(file.fileName());

    if (!prefix.isEmpty())
    {
        return newPrefixedFileNamePath(file, destDir, prefix);
    }

    return {QString(), FileName::Status::NotAvailable};
}
