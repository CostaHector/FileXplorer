#include "VidsDurationDisplayString.h"
#include "VideoDurationGetter.h"
#include <QTime>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QFileInfo>

QString VidsDurationDisplayString::DisplayVideosDuration(const QStringList& fileAbsPaths) {
  VideoDurationGetter mi;
  if (!mi.StartToGet()) {
    return {};
  }
  const QList<int>& durationLst = mi.GetLengthsQuick(fileAbsPaths);
  return DurationPrepathName2Table(durationLst, fileAbsPaths);
}

QString VidsDurationDisplayString::DurationPrepathName2Table(const QList<int>& durationLst, const QStringList& fileAbsPaths) {
  QStringList fileNames, fileDirs;
  fileNames.reserve(durationLst.size());
  fileDirs.reserve(durationLst.size());
  for (const auto& pth : fileAbsPaths) {
    const QFileInfo fi{pth};
    fileNames.append(fi.fileName());
    fileDirs.append(fi.absolutePath());
  }
  return VideosDurationDetailHtmlTable(durationLst, fileNames, fileDirs);
}

QString VidsDurationDisplayString::VideosDurationDetailHtmlTable(const QList<int>& durationLst,
                                                                 const QStringList& fileNames,
                                                                 const QStringList& fileDirs) {
  if (not(durationLst.size() == fileNames.size() and fileNames.size() == fileDirs.size())) {
    qWarning("list length unequal. duration[%d], fileName[%d], fileDirs[%d]", durationLst.size(), fileNames.size(), fileDirs.size());
    return "";
  }
  static const QString& DURATION_TABLE_TEMPLATE{
      "<table>\n"
      "<caption>Durations Details</caption>\n"
      "<tr>\n"
      "<th style=\"border-right:2px solid red\">Duration</th>\n"
      "<th style=\"border-right:2px solid red\">Name</th>\n"
      "<th>Path</th>\n"
      "</tr>\n"
      "%1"
      "\n"
      "</table>"};
  static const QString& DURATION_TABLE_ROW_TEMPLATE{
      "\n"
      "<tr>\n"
      "<td style=\"border-right:2px solid red\">%1</td>\n"
      "<td style=\"border-right:2px solid red\">%2</td>\n"
      "<td>%3</td>\n"
      "</tr>\n"};

  QString rows;
  unsigned long totalLength = 0;
  for (int i = 0; i < durationLst.size(); ++i) {
    totalLength += durationLst[i] / 1000;
    rows += DURATION_TABLE_ROW_TEMPLATE.arg(QTime::fromMSecsSinceStartOfDay(durationLst[i]).toString(Qt::ISODateWithMs))
                .arg(fileNames[i])
                .arg(fileDirs[i]);
  }
  return QString("Total duration: %1(s) of %2 video(s)").arg(totalLength).arg(durationLst.size()) + DURATION_TABLE_TEMPLATE.arg(rows);
}

#include <QCoreApplication>

// download 64bit zip DLL	v24.04 (without installer: 7z, zip) from
// https://mediaarea.net/en/MediaInfo/Download/Windows
// https://github.com/sylvrec/QMediaInfo
// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
int main(int argc, char* argv[]) {
  QMediaInfo mi;
  mi.Open("path_2_a_video_here.mp4");
  QString dur = mi.Duration();
  int len = mi.DurationLength();
  qDebug("duration: %s, length: %d", qPrintable(dur), len);
  return 0;
}
#endif
