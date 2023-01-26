#include "nomoremess.hpp"
#include "processingworker.hpp"
#include "statistics.hpp"
#include "ui_nomoremess.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QThread>
#include <QtConcurrent/QtConcurrent>

#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

namespace
{
QString prepareStatisticsLabelText(int filesCnt, double size)
{
    return QStringLiteral("Files: %1\nSize: %2 MB").arg(filesCnt).arg(size, 0, 'f', 2);
}

QString prepareProgressLabelText(int filesCnt, int maxFilesCnt, double size, double maxSize)
{
    return QStringLiteral("Files: %1/%2\nSize: %3/%4 MB")
            .arg(filesCnt)
            .arg(maxFilesCnt)
            .arg(size, 0, 'f', 2)
            .arg(maxSize, 0, 'f', 2);
}

void saveReport(const QString &destination, const Report &report)
{
    QDir dir(destination);
    const auto reportFileName = dir.absoluteFilePath("report.txt");

    QFile::remove(reportFileName);

    QFile reportFile(reportFileName);
    if (!reportFile.open(QIODevice::ReadWrite))
    {
        QMessageBox::critical(nullptr, "Can't store report!!", report.uncopiedFiles.join('\n'));
    }

    QTextStream stream(&reportFile);
    stream << "Status: " << (report.finished ? "FINISHED" : "UNFINISHED!!!") << '\n';
    stream << "Overall files count:" << report.filesCount << '\n';
    stream << "Overall files size:" << report.size << '\n';

    stream << "Copied files size:  " << report.copiedSize << '\n';
    stream << "Copied files: " << report.copiedFiles.size() << '\n';
    for (const auto &name : report.copiedFiles)
    {
        stream << "-" << name << '\n';
    }
    stream << "========\n";

    stream << "Already existing:   " << report.alreadyExisting << '\n';

    stream << "Renamed:\n";
    for (const auto &name : report.renamed)
    {
        stream << "-" << name << '\n';
    }
    stream << "========\n";

    stream << "Uncopied files size:" << report.uncopiedSize << '\n';
    stream << "Uncopied files:" << report.uncopiedFiles.size() << "\n";
    for (const auto &name : report.uncopiedFiles)
    {
        stream << "-" << name << '\n';
    }
    stream << "========\n";

    stream << "Unhandled files size:" << report.unhandledSize << '\n';
    stream << "Unhandled files:" << report.unhandledFiles.size() << "\n";
    for (const auto &name : report.unhandledFiles)
    {
        stream << "-" << name << '\n';
    }
    stream << "========\n";

    stream.flush();
    reportFile.close();
}

} // namespace

NoMoreMess::NoMoreMess(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::NoMoreMess)
        , m_processThread(new QThread(this))
        , mWorker(nullptr)
        , m_progressBar(nullptr)
{
    qRegisterMetaType<Stats>();
    ui->setupUi(this);

    QPieSeries *series = new QPieSeries();
    series->append("Images", 0);
    series->append("Videos", 0);
    series->append("Other", 0);

    QChart *chart = new QChart();
    chart->setAnimationDuration(2000);
    chart->setAnimationEasingCurve(QEasingCurve(QEasingCurve::InOutBounce));
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->addSeries(series);
    //    chart->setTitle("Simple piechart example");
    chart->legend()->hide();

    ui->widget->setRenderHint(QPainter::Antialiasing);
    ui->widget->setChart(chart);
}

NoMoreMess::~NoMoreMess()
{
    delete ui;
}

void NoMoreMess::on_openPathButton_released()
{
    const QString dir = QFileDialog::getExistingDirectory(
            this,
            tr("Open Directory"),
            "",
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
        return;

    mSettings.sourcePath = dir;

    ui->pathLineEdit->setText(dir);
}

void NoMoreMess::setRange(int max)
{
    m_progressBar->setRange(0, max);
}

void NoMoreMess::setValue(int val)
{
    if (m_progressBar)
        m_progressBar->setValue(val);
}

void NoMoreMess::finish(const Stats &stats)
{
    if (m_progressBar)
    {
        m_progressBar->deleteLater();
        m_progressBar = nullptr;
    }

    ui->label->setText(QString::number(stats.m_imagesSize));
    auto imgs = dynamic_cast<QPieSeries *>(ui->widget->chart()->series().front())->slices().front();
    imgs->setValue(stats.m_imagesSize);
    auto oo = dynamic_cast<QPieSeries *>(ui->widget->chart()->series().front())->slices().at(2);
    oo->setValue(stats.m_imagesSize / 2);
}

void NoMoreMess::on_reloadStatisticsButton_released()
{
    m_progressBar = new QProgressDialog("Collecting statistics", QString(), 0, 0, this);
    m_progressBar->open();
    emit read_stats(ui->pathLineEdit->text(), ui->subdirsCheckBox->isChecked());
}

void NoMoreMess::on_doMagicButton_released()
{
    mWorker = new ProcessingWorker(mSettings);
    mWorker->moveToThread(m_processThread);

    m_progressBar = new QProgressDialog("Collecting statistics", "Cancel", 0, 0, this);
    m_progressBar->setWindowModality(Qt::WindowModal);

    connect(m_processThread, &QThread::started, mWorker, &ProcessingWorker::process);
    //    connect(mWorker, &ProcessingWorker::done, m_processThread, &QThread::quit);
    connect(mWorker, &ProcessingWorker::done, this, [&](const Report &report) {
        m_processThread->quit();
        saveReport(mSettings.destinationPath, report);
        //        QMessageBox::information(this, "Info", "Processing finished");
    });
    connect(mWorker,
            &ProcessingWorker::calculationProgressChanged,
            m_progressBar,
            [&](int filesCount, double size, bool finished) {
                m_progressBar->setLabelText(prepareStatisticsLabelText(filesCount, size));
                if (finished)
                {
                    mSize = size;
                    mFilesCount = filesCount;
                    m_progressBar->setMaximum(mSize);
                }
            });
    //    connect(mWorker, &ProcessingWorker::progressChanged, m_progressBar,
    //    &QProgressDialog::setValue);
    connect(mWorker,
            &ProcessingWorker::progressChanged,
            m_progressBar,
            [&](int count, double size, bool finished) {
                //        qDebug() << "PROGRESS:" << progress;
                m_progressBar->setLabelText(
                        prepareProgressLabelText(count, mFilesCount, size, mSize));
                m_progressBar->setValue(size);
            });
    connect(m_progressBar,
            &QProgressDialog::canceled,
            mWorker,
            &ProcessingWorker::stop,
            Qt::DirectConnection);
    connect(m_processThread, &QThread::finished, m_progressBar, &QProgressDialog::close);
    connect(m_processThread, &QThread::finished, mWorker, &ProcessingWorker::deleteLater);
    connect(m_progressBar,
            &QProgressDialog::finished,
            m_progressBar,
            &QProgressDialog::deleteLater);

    m_progressBar->open();
    m_processThread->start();

    qDebug() << "TEST";
}

void NoMoreMess::on_openPathButton_2_released()
{
    const QString dir = QFileDialog::getExistingDirectory(
            this,
            tr("Open Directory"),
            "",
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
        return;

    mSettings.destinationPath = dir;

    ui->destinationPathLineEdit->setText(dir);
}
