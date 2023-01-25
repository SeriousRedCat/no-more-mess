#ifndef NOMOREMESS_HPP
#define NOMOREMESS_HPP

#include <QMainWindow>

#include "settings.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class NoMoreMess;
}
QT_END_NAMESPACE

class QThread;
class QProgressDialog;
struct Stats;

class NoMoreMess : public QMainWindow {
  Q_OBJECT

public:
  NoMoreMess(QWidget *parent = nullptr);
  ~NoMoreMess();

private slots:
  void on_openPathButton_released();
  void setRange(int max);
  void setValue(int val);
  void finish(const Stats &stats);

  void on_reloadStatisticsButton_released();

  void on_doMagicButton_released();

signals:
  void read_stats(const QString &dir, bool recursive);
  void process(const QString &dir, bool recursive);

private:
  Ui::NoMoreMess *ui;
  QThread *m_statsThread;
  //  QThread *m_processThread;
  Settings mSettings;

  QProgressDialog *m_progressBar;
};
#endif // NOMOREMESS_HPP
