#ifndef VIDSDURATIONDISPLAYSTRING_H
#define VIDSDURATIONDISPLAYSTRING_H
#include <QStringList>
#include <QList>

namespace VidsDurationDisplayString {
#ifdef RUNNING_UNIT_TESTS
inline QList<int>& MockFilesDurationLstReturn() {
  static QList<int> durationLst;
  return durationLst;
}
#endif

QString DisplayVideosDuration(const QStringList& fileAbsPaths);
QString DurationPrepathName2Table(const QList<int>& durationLst, const QStringList& fileAbsPaths);
QString VideosDurationDetailHtmlTable(const QList<int>& durationLst, const QStringList& fileNames, const QStringList& fileDirs);
}  // namespace MP4DurationGetter

#endif  // VIDSDURATIONDISPLAYSTRING_H
