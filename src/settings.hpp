#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <QString>
#include <QStringList>

struct Settings
{
    QString sourcePath;
    QString destinationPath;
    bool recursive;
};

struct Report
{
    int filesCount = 0;
    double size = 0.;

    QStringList copiedFiles;
    double copiedSize = 0.;

    int alreadyExisting = 0;

    QStringList renamed;

    QStringList uncopiedFiles;
    double uncopiedSize = 0.;

    QStringList unhandledFiles;
    double unhandledSize = 0.;

    //    QStringList uncopiedFileNames;

    bool finished = false;
};

#endif // SETTINGS_HPP
