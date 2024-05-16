#include "VidsDurationDisplayString.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QFileInfo>
#include "QMediaInfo.h"

QString VidsDurationDisplayString::DisplayVideosDuration(const QList<int>& durationLst, const QStringList& fileNames, const QStringList& fileDirs) {
  if (not(durationLst.size() == fileNames.size() and fileNames.size() == fileDirs.size())) {
    qDebug("list size must same[%d, %d, %d]", durationLst.size(), fileNames.size(), fileDirs.size());
    return "";
  }
  unsigned long totalLength = 0;
  QString dispMsg;
  for (int i = 0; i < durationLst.size(); ++i) {
    totalLength += durationLst[i];
    dispMsg += (QString::number(durationLst[i]) + '\t' + fileNames[i] + '\t' + fileDirs[i] + '\n');
  }
  return QString("Total duration:\n%1(s) of %2 video(s)\n").arg(totalLength).arg(durationLst.size()) + dispMsg;
}

QString VidsDurationDisplayString::DisplayVideosDuration(const QList<int>& durationLst, const QStringList& fileAbsPaths) {
  QStringList fileNames, fileDirs;
  for (const auto& pth : fileAbsPaths) {
    QFileInfo fi(pth);
    fileNames << fi.fileName();
    fileDirs << fi.absolutePath();
  }
  return DisplayVideosDuration(durationLst, fileNames, fileDirs);
}

QString VidsDurationDisplayString::DisplayVideosDuration(const QStringList& fileAbsPaths) {
  QMediaInfo mi;
  const QList<int>& durationLst = mi.batchVidsDurationLength(fileAbsPaths);
  return DisplayVideosDuration(durationLst, fileAbsPaths);
}

#include <QCoreApplication>

// download 64bit zip DLL	v24.04 (without installer: 7z, zip) from
// https://mediaarea.net/en/MediaInfo/Download/Windows
// https://github.com/sylvrec/QMediaInfo
//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
int main(int argc, char* argv[]) {
  QMediaInfo mi;
  mi.Open("E:/115/0419/Axel Rockham/KB Axel Rockham Magnus Loki Lucio Saints 1080p.mp4");
  QString dur = mi.Duration();
  int len = mi.DurationLength();
  qDebug("duration: %s, length: %d", qPrintable(dur), len);
  return 0;
}
#endif
