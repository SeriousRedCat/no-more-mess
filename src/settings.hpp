#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <QString>

struct Settings {
  QString sourcePath;
  QString destinationPath;
  bool recursive;
};

#endif // SETTINGS_HPP
