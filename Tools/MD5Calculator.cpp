#include "MD5Calculator.h"
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>

QString MD5Calculator::GetMD5(const QString& filepath, const int onlyFirstByte) {
  QFile file(filepath);
  if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
    qDebug("file[%s] open failed or not exist", qPrintable(filepath));
    return "";
  }
  QCryptographicHash md5(QCryptographicHash::Md5);
  if (onlyFirstByte > 0) {
    md5.addData(file.read(onlyFirstByte));
  } else {
    while (!file.atEnd()) {
      md5.addData(file.read(8192));
    }
  }
  file.close();
  return md5.result().toHex();
}

QStringList MD5Calculator::GetBatchMD5(const QStringList& filepaths, const int /* onlyFirstByte */) {
  QStringList md5Lst;
  md5Lst.reserve(filepaths.size());
  for (const auto& path : filepaths) {
    QFileInfo fi(path);
    if (not fi.exists() or not fi.isFile()) {
      continue;
    }
    md5Lst << GetMD5(path);
  }
  return md5Lst;
}

QString MD5Calculator::DisplayFilesMD5(const QStringList& fileAbsPaths) {
  return MD5PrepathName2Table(GetBatchMD5(fileAbsPaths), fileAbsPaths);
}

QString MD5Calculator::MD5PrepathName2Table(const QStringList& md5s, const QStringList& fileAbsPaths) {
  QStringList fileNames, fileDirs;
  for (const auto& pth : fileAbsPaths) {
    QFileInfo fi(pth);
    if (not fi.exists() or not fi.isFile()) {
      continue;
    }
    fileNames << fi.fileName();
    fileDirs << fi.absolutePath();
  }
  return QString("MD5 of %1 file(s)\n").arg(md5s.size()) + MD5DetailHtmlTable(md5s, fileNames, fileDirs);
}

QString MD5Calculator::MD5DetailHtmlTable(const QStringList& md5s, const QStringList& fileNames, const QStringList& fileDirs) {
  if (not(md5s.size() == fileNames.size() and fileNames.size() == fileDirs.size())) {
    qWarning("list length unequal. md5s[%d], fileName[%d], fileDirs[%d]", md5s.size(), fileNames.size(), fileDirs.size());
    return "";
  }
  const QString& MD5_TABLE_TEMPLATE{
      "<table>\n"
      "<caption>File Identifiers Info</caption>\n"
      "<tr>\n"
      "<th style=\"border-right:2px solid red\">MD5</th>\n"
      "<th style=\"border-right:2px solid red\">Name</th>\n"
      "<th>Path</th>\n"
      "</tr>\n"
      "%1"
      "\n"
      "</table>"};
  const QString& MD5_TABLE_ROW_TEMPLATE{
      "\n"
      "<tr>\n"
      "<td style=\"border-right:2px solid red\">%1</td>\n"
      "<td style=\"border-right:2px solid red\">%2</td>\n"
      "<td>%3</td>\n"
      "</tr>\n"};
  QString rows;
  for (int i = 0; i < md5s.size(); ++i) {
    rows += MD5_TABLE_ROW_TEMPLATE.arg(md5s[i]).arg(fileNames[i]).arg(fileDirs[i]);
  }
  return MD5_TABLE_TEMPLATE.arg(rows);
}
