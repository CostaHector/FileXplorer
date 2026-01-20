#ifndef VIDEODURATIONGETTERMOCK_H
#define VIDEODURATIONGETTERMOCK_H
#include "VideoDurationGetter.h"

namespace VideoDurationGetterMock {

void PresetVidsDuration(QHash<QString, int> vidPath2Duration);

int invokeReadAVideo(const QString& vidPath);
QList<int> invokeReadVideos(const QStringList& vidsPath);
int invokeGetLengthQuickStatic(const VideoDurationGetter& self, const QString& vidPath);
QList<int> invokeGetLengthsQuickStatic(const VideoDurationGetter& self, const QStringList& vidsPath);
}

#endif // VIDEODURATIONGETTERMOCK_H
