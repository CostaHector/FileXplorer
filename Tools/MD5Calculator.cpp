#include "MD5Calculator.h"
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>

QString MD5Calculator::getMd5(const QString& filepath) {
  QFile file(filepath);
  file.open(QIODevice::ReadOnly);
  QCryptographicHash md5(QCryptographicHash::Md5);
  while (!file.atEnd()) {
    md5.addData(file.read(8192));
  }
  file.close();
  return md5.result().toHex();
}

QStringList MD5Calculator::getBatchMD5(const QStringList& filepaths) {
  QStringList md5Lst;
  md5Lst.reserve(filepaths.size());
  for (const auto& path : filepaths) {
    QFileInfo fi(path);
    if (not fi.exists() or not fi.isFile()) {
      continue;
    }
    md5Lst << getMd5(path);
  }
  return md5Lst;
}

QString MD5Calculator::DisplayFilesMD5(const QStringList& md5s, const QStringList& fileNames, const QStringList& fileDirs) {
  if (not(md5s.size() == fileNames.size() and fileNames.size() == fileDirs.size())) {
    qDebug("list size must same[%d, %d, %d]", md5s.size(), fileNames.size(), fileDirs.size());
    return "";
  }
  QString dispMsg;
  for (int i = 0; i < md5s.size(); ++i) {
    dispMsg += (md5s[i] + '\t' + fileNames[i] + '\t' + fileDirs[i] + '\n');
  }
  return dispMsg;
}

QString MD5Calculator::DisplayFilesMD5(const QStringList& md5s, const QStringList& fileAbsPaths) {
  QStringList fileNames, fileDirs;
  for (const auto& pth : fileAbsPaths) {
    QFileInfo fi(pth);
    if (not fi.exists() or not fi.isFile()) {
      continue;
    }
    fileNames << fi.fileName();
    fileDirs << fi.absolutePath();
  }
  return QString("MD5 of %1 file(s)\n").arg(md5s.size()) + DisplayFilesMD5(md5s, fileNames, fileDirs);
}

QString MD5Calculator::DisplayFilesMD5(const QStringList& fileAbsPaths) {
  return DisplayFilesMD5(getBatchMD5(fileAbsPaths), fileAbsPaths);
}
