#include "PlayVideo.h"

PlayVideo::PlayVideo() {}

bool PlayADir(const QString& dirPath) {
  //    if (not NRF.isAvail("VideoPlayer")){
  //        return false;
  //    }
  //    const QString& exePath = NRF.GetUrl("VideoPlayer");
  const QString& exePath = "C:/Program Files/DAUM/PotPlayer/PotPlayerMini64.exe";
  QFileInfo playerFi(exePath);
  QProcess process;
#ifdef _WIN32
  process.setProgram(playerFi.absoluteFilePath());
  process.setArguments({QFileInfo(dirPath).absoluteFilePath()});
  process.startDetached();  // Start the process in detached mode instead of start
  return true;
#else
  qDebug("Not on WIN32 platform");
  return false;
#endif
}

bool on_ShiftEnterPlayVideo(QString& path) {
  if (not QFile::exists(path)) {
    return false;
  }
  QFileInfo fi(path);
  if (fi.isDir()) {
    if (PlayADir(path)) {  // try play it now
      return true;
    }
    // try to find a vids
    QDir dir(fi.absoluteFilePath());
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::SortFlag::Name | QDir::SortFlag::IgnoreCase);
    dir.setNameFilters(TYPE_FILTER::VIDEO_TYPE_SET);
    QList<QString> fiList = dir.entryList();
    if (fiList.isEmpty()) {
      return false;
    }
    fi = QFileInfo(dir.absoluteFilePath(fiList.back()));
  } else if (fi.isFile()) {
    if (fi.isSymLink()) {
      fi = QFileInfo(fi.symLinkTarget());
    }
  } else {
    qDebug("[Error] Never goes here");
    return false;
  }

  if (fi.isFile() and (TYPE_FILTER::VIDEO_TYPE_SET.contains("*." + fi.completeSuffix()))) {
    return QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
  }
  return false;
}
