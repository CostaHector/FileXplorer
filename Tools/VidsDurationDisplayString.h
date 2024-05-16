#ifndef VIDSDURATIONDISPLAYSTRING_H
#define VIDSDURATIONDISPLAYSTRING_H
#include <QStringList>
#include <QList>

namespace VidsDurationDisplayString {
QString DisplayVideosDuration(const QList<int>& durationLst, const QStringList& fileNames, const QStringList& fileDirs);

QString DisplayVideosDuration(const QList<int>& durationLst, const QStringList& fileAbsPaths);

QString DisplayVideosDuration(const QStringList& fileAbsPaths);
}  // namespace MP4DurationGetter

#endif  // VIDSDURATIONDISPLAYSTRING_H
