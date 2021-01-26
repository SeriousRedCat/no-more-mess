#include "nomoremess.hpp"
#include "ui_nomoremess.h"
#include "statistics.hpp"

#include <QtConcurrent/QtConcurrent>
#include <QDir>
#include <QFileDialog>
#include <QThread>
#include <QProgressDialog>

NoMoreMess::NoMoreMess(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::NoMoreMess),
    m_statsThread(new QThread(this)),
    m_progressBar(nullptr)
{
    ui->setupUi(this);

    Statistics* stats = new Statistics();
    stats->moveToThread(m_statsThread);

    QObject::connect(this,  &NoMoreMess::read_stats, stats, &Statistics::calculateStatistics);

    QObject::connect(stats, &Statistics::newRange, this, &NoMoreMess::setRange);
    QObject::connect(stats, &Statistics::progressChanged, this, &NoMoreMess::setValue);
    QObject::connect(stats, &Statistics::finished, this, &NoMoreMess::finish);
    m_statsThread->start();
}

NoMoreMess::~NoMoreMess()
{
    m_statsThread->quit();
    m_statsThread->wait();
    delete ui;
}


void NoMoreMess::on_openPathButton_released()
{
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "",
                                                    QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);

    if(dir.isEmpty())
        return;

    ui->pathLineEdit->setText(dir);
}

void NoMoreMess::setRange(int max)
{
    m_progressBar->setRange(0,max);
}

void NoMoreMess::setValue(int val)
{
    if(m_progressBar)
        m_progressBar->setValue(val);
}

void NoMoreMess::finish(const Stats& stats)
{
    if(m_progressBar) {
        m_progressBar->deleteLater();
        m_progressBar = nullptr;
    }
}

void NoMoreMess::on_reloadStatisticsButton_released()
{
    m_progressBar = new QProgressDialog("Collecting statistics", "Cancel", 0, 0, this);
    m_progressBar->open();
    emit read_stats(ui->pathLineEdit->text(), ui->subdirsCheckBox->isChecked());
}
