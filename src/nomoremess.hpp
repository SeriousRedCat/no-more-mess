#ifndef NOMOREMESS_HPP
#define NOMOREMESS_HPP

#include <QMainWindow>

#include "processingworker.hpp"
#include "settings.hpp"

QT_BEGIN_NAMESPACE
namespace Ui
{
class NoMoreMess;
}
QT_END_NAMESPACE

class QThread;
class QProgressDialog;
struct Stats;

class NoMoreMess : public QMainWindow
{
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

    void on_openPathButton_2_released();

signals:
    void read_stats(const QString &dir, bool recursive);
    void process(const QString &dir, bool recursive);

private:
    Ui::NoMoreMess *ui;
    QThread *m_processThread;
    ProcessingWorker *mWorker;
    Settings mSettings;

    int mFilesCount = 0;
    double mSize = 0.;

    QProgressDialog *m_progressBar;
};
#endif // NOMOREMESS_HPP
