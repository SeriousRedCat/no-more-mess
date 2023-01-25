#ifndef PROCESSINGWORKER_HPP
#define PROCESSINGWORKER_HPP

#include <QObject>
#include <memory>

#include <QThread>

#include "settings.hpp"

class ProcessingWorker : public QObject {
  Q_OBJECT
public:
  ProcessingWorker(const Settings &settings);
  ~ProcessingWorker();

  void process();

signals:
  void progressChanged(int files, int size);

public slots:
  void stop();

private slots:
  void cleanup();

private:
  std::unique_ptr<QThread> mThread;
  const Settings mSettings;
};

#endif // PROCESSINGWORKER_HPP
