#include "nomoremess.hpp"
#include "processingworker.hpp"
#include "statistics.hpp"
#include "ui_nomoremess.h"

#include <QDir>
#include <QFileDialog>
#include <QProgressDialog>
#include <QThread>
#include <QtConcurrent/QtConcurrent>

#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

NoMoreMess::NoMoreMess(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::NoMoreMess),
      m_statsThread(new QThread(this)), m_progressBar(nullptr) {
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

  Statistics *stats = new Statistics();
  stats->moveToThread(m_statsThread);

  QObject::connect(this, &NoMoreMess::read_stats, stats,
                   &Statistics::calculateStatistics);

  QObject::connect(stats, &Statistics::newRange, this, &NoMoreMess::setRange);
  QObject::connect(stats, &Statistics::progressChanged, this,
                   &NoMoreMess::setValue);
  QObject::connect(stats, &Statistics::finished, this, &NoMoreMess::finish);
  m_statsThread->start();

  //   ProcessingWorker* processingW = new ProcessingWorker();
  //   processingW->moveToThread(m_processThread);
  //   m_processThread->start();
}

NoMoreMess::~NoMoreMess() {
  m_statsThread->quit();
  m_statsThread->wait();

  delete ui;
}

void NoMoreMess::on_openPathButton_released() {
  const QString dir = QFileDialog::getExistingDirectory(
      this, tr("Open Directory"), "",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  if (dir.isEmpty())
    return;

  ui->pathLineEdit->setText(dir);
}

void NoMoreMess::setRange(int max) { m_progressBar->setRange(0, max); }

void NoMoreMess::setValue(int val) {
  if (m_progressBar)
    m_progressBar->setValue(val);
}

void NoMoreMess::finish(const Stats &stats) {
  if (m_progressBar) {
    m_progressBar->deleteLater();
    m_progressBar = nullptr;
  }

  ui->label->setText(QString::number(stats.m_imagesSize));
  auto imgs = dynamic_cast<QPieSeries *>(ui->widget->chart()->series().front())
                  ->slices()
                  .front();
  imgs->setValue(stats.m_imagesSize);
  auto oo = dynamic_cast<QPieSeries *>(ui->widget->chart()->series().front())
                ->slices()
                .at(2);
  oo->setValue(stats.m_imagesSize / 2);
}

void NoMoreMess::on_reloadStatisticsButton_released() {
  m_progressBar =
      new QProgressDialog("Collecting statistics", "Cancel", 0, 0, this);
  m_progressBar->open();
  emit read_stats(ui->pathLineEdit->text(), ui->subdirsCheckBox->isChecked());
}

void NoMoreMess::on_doMagicButton_released() {
  ProcessingWorker worker(mSettings);
}
