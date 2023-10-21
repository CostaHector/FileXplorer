#ifndef PLAYVIDEO_H
#define PLAYVIDEO_H

class PlayVideo {
 public:
  PlayVideo();
};

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QString>
#include <QUrl>

#include "PublicVariable.h"

bool PlayADir(const QString& dirPath);

bool on_ShiftEnterPlayVideo(QString& path);

#endif  // PLAYVIDEO_H
