#ifndef STATISTICS_HPP
#define STATISTICS_HPP

#include <QDir>
#include <QObject>

struct Stats {
    unsigned int m_filesCnt = 0;
    double m_imagesSize = 0.;
    double m_videosSize = 0.;
    double m_othersSize = 0.;
};

Q_DECLARE_METATYPE(Stats)

class Statistics : public QObject
{
    Q_OBJECT
public:
    Statistics();
    virtual ~Statistics() override = default;

signals:
    void newRange(int max);
    void progressChanged(int val);
    void finished(const Stats& stats);

public slots:
    void calculateStatistics(const QString& path, bool recursive);

private:
    int count(const QDir& dir, bool recursive);
};

#endif // STATISTICS_HPP
