#include "statistics.hpp"

#include <QDirIterator>
#include <QThread>
#include <QDebug>

QSet<QString> images{
    "jpg",
    "jpeg"
};

Statistics::Statistics():
    QObject(nullptr)
{

}

void Statistics::calculateStatistics(const QString &path, bool recursive)
{
    QDir aaa(path);
    aaa.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    emit newRange(count(aaa, recursive));

    QDirIterator it2(aaa, recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);

    int i = 0;
    Stats stats;

    while(it2.hasNext())
    {
        QString name = it2.next();
        QFileInfo info(name);

        if(images.contains(info.suffix().toLower())) {
            stats.m_imagesSize += info.size();
        } else {
            ++stats.m_othersSize += info.size();
        }
        emit progressChanged(++i);
//        QThread::msleep(1);
    }

    emit finished(stats);
}

int Statistics::count(const QDir &dir, bool recursive)
{
    if(recursive) {
        QDirIterator it(dir, QDirIterator::Subdirectories);
        int cnt = 0;
        while(it.hasNext()) {
            it.next();
            ++cnt;
        }
        return cnt;
    }
    else {
        return dir.count();
    }
}
