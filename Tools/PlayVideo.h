#ifndef PLAYVIDEO_H
#define PLAYVIDEO_H


class PlayVideo {
public:
    PlayVideo();
};

#include <QString>
#include <QFileInfo>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>

#include "PublicVariable.h"

bool PlayADir(const QString& dirPath);


bool on_ShiftEnterPlayVideo(QString& path);


#endif // PLAYVIDEO_H
