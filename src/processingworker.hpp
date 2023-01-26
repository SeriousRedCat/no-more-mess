#ifndef PROCESSINGWORKER_HPP
#define PROCESSINGWORKER_HPP

#include <QObject>
#include <memory>

#include <QFileInfo>
#include <QMutex>
#include <QThread>

#include "settings.hpp"

class ProcessingWorker : public QObject
{
    Q_OBJECT
public:
    ProcessingWorker(const Settings &settings);
    ~ProcessingWorker();

    void process();

signals:
    void calculationProgressChanged(int files, double size, bool finished);
    void progressChanged(int files, double size, bool finished);
    void done(const Report &report);

public slots:
    void stop();

private slots:
    void cleanup();

private:
    void run();
    void grabStatistics();
    bool isStopped();

    struct FileName
    {
        enum class Status
        {
            Ok,
            Sufixxed,
            Duplicated,
            NotAvailable
        };

        QString name;
        Status status;
    };

    FileName generateNewFileName(const QFileInfo &file, const QDir &destDir);

    FileName newPrefixedFileNamePath(
            const QFileInfo &currentFile, const QDir &destDir, const QString &prefix);

    bool mStopped;
    QMutex mMutex;

    //    std::unique_ptr<QThread> mThread;
    const Settings mSettings;
    Report mReport;
};

#endif // PROCESSINGWORKER_HPP
