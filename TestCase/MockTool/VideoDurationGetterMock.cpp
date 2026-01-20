#include "VideoDurationGetterMock.h"
#include <QHash>
namespace {
  QHash<QString, int> g_vidPath2Duration;
}

namespace VideoDurationGetterMock {

void PresetVidsDuration(QHash<QString, int> vidPath2Duration) {
  g_vidPath2Duration.swap(vidPath2Duration);
}


int invokeReadAVideo(const QString& vidPath) {
  return g_vidPath2Duration.value(vidPath, -1);
}

QList<int> invokeReadVideos(const QStringList& vidsPath) {
  QList<int> ansList;
  ansList.reserve(vidsPath.size());
  for (const QString& vidPath: vidsPath) {
    ansList.push_back(g_vidPath2Duration.value(vidPath, -1));
  }
  return ansList;
}

int invokeGetLengthQuickStatic(const VideoDurationGetter& self, const QString& vidPath) {
  return g_vidPath2Duration.value(vidPath, -1);
}
QList<int> invokeGetLengthsQuickStatic(const VideoDurationGetter& self, const QStringList& vidsPath) {
  QList<int> ansList;
  ansList.reserve(vidsPath.size());
  for (const QString& vidPath: vidsPath) {
    ansList.push_back(g_vidPath2Duration.value(vidPath, -1));
  }
  return ansList;
}

}
