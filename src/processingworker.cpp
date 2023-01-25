#include "processingworker.hpp"

ProcessingWorker::ProcessingWorker(const Settings &settings)
    : mSettings(settings) {
  mThread = std::make_unique<QThread>();
  moveToThread(mThread.get());
}

ProcessingWorker::~ProcessingWorker() {
  QMetaObject::invokeMethod(this, "cleanup");
  mThread->wait();
}

void ProcessingWorker::process() { mThread->start(); }

void ProcessingWorker::stop() {}

void ProcessingWorker::cleanup() { mThread->quit(); }
