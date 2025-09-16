#include "MD5Calculator.h"
#include "Logger.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QFileInfo>

namespace MD5Calculator {

QString GetFileMD5(const QString& filepath, const int onlyFirstByte, QCryptographicHash::Algorithm alg) {
  QFile file{filepath};
  if (!file.exists()) {
    LOG_W("file[%s] not found", qPrintable(filepath));
    return "";
  }
  if (!file.open(QIODevice::ReadOnly)) {
    LOG_D("file[%s] open failed", qPrintable(filepath));
    return "";
  }
  if (onlyFirstByte > 0) {
    const QString ans = GetByteArrayMD5(file.read(onlyFirstByte), alg);
    file.close();
    return ans;
  }
  QCryptographicHash md5(alg);
  while (!file.atEnd()) {
    md5.addData(file.read(8192));
  }
  file.close();
  return md5.result().toHex();
}

QString GetByteArrayMD5(const QByteArray& ba, QCryptographicHash::Algorithm alg) {
  QCryptographicHash md5(QCryptographicHash::Md5);
  md5.addData(ba);
  return md5.result().toHex();
}

QStringList GetBatchFileMD5(const QStringList& filepaths, const int onlyFirstByte, QCryptographicHash::Algorithm alg) {
  QStringList md5Lst;
  md5Lst.reserve(filepaths.size());
  for (const auto& path : filepaths) {
    QFileInfo fi(path);
    if (!fi.exists() || !fi.isFile()) {
      continue;
    }
    md5Lst << GetFileMD5(path, onlyFirstByte, alg);
  }
  return md5Lst;
}

QString DisplayFilesMD5(const QStringList& fileAbsPaths) {
  return MD5PrepathName2Table(GetBatchFileMD5(fileAbsPaths), fileAbsPaths);
}

QString MD5PrepathName2Table(const QStringList& md5s, const QStringList& fileAbsPaths) {
  QStringList fileNames, fileDirs;
  for (const auto& pth : fileAbsPaths) {
    const QFileInfo fi{pth};
    if (!fi.exists() or !fi.isFile()) {
      continue;
    }
    fileNames << fi.fileName();
    fileDirs << fi.absolutePath();
  }
  return QString("MD5 of %1 file(s)\n").arg(md5s.size()) + MD5DetailHtmlTable(md5s, fileNames, fileDirs);
}

QString MD5DetailHtmlTable(const QStringList& md5s, const QStringList& fileNames, const QStringList& fileDirs) {
  if (not(md5s.size() == fileNames.size() and fileNames.size() == fileDirs.size())) {
    LOG_W("list length unequal. md5s[%d], fileName[%d], fileDirs[%d]", md5s.size(), fileNames.size(), fileDirs.size());
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

QString GetHashPlatformDependent(const QString& absFilePath) {
  if (!QFileInfo::exists(absFilePath)) {
    LOG_W("File[%s] not exist at all", qPrintable(absFilePath));
    return "";
  }

  QString cmd;
  QStringList args;
  args.reserve(3);
#ifdef _WIN32
  cmd = "CertUtil";
  args << "-hashfile" << QDir::toNativeSeparators(absFilePath) << "MD5";
#elif defined(__linux__)
  cmd = "md5sum";
  args << QDir::toNativeSeparators(absFilePath);
#else
  LOG_W("unsupport platform");
  return "";
#endif

  QProcess process;
  process.start(cmd, args);

  if (!process.waitForFinished(5000)) {
    LOG_W("command execute failed[%s]", qPrintable(process.errorString()));
    return "";
  }

  if (process.exitCode() != 0) {
    LOG_W("command error[%s]", qPrintable(process.readAllStandardError()));
    return "";
  }

  // Parse the output message
  QString output = QString::fromUtf8(process.readAllStandardOutput());
  QString hashResult;

#ifdef _WIN32
  // Windows output format pattern: the second line
  /* MD5 hash of CoverageHelper.md:
     a94a8fe5ccb19ba61c4c0873d391e987982fbbd3
     CertUtil: -hashfile command completed successfully. */
  int beforeHashStartIndex = output.indexOf('\n');
  if (beforeHashStartIndex == -1) {
    LOG_W("output message[%s] invalid", qPrintable(output));
    return "";
  }
  return output.mid(beforeHashStartIndex + 1, 32);
  // QRegularExpression re("([a-f0-9]{32})", QRegularExpression::CaseInsensitiveOption);
  // QRegularExpressionMatch match = re.match(output);
  // if (match.hasMatch()) {
  //   hashResult = match.captured(1).toLower();
  // }
#elif defined(__linux__)
  // Linux output format pattern:
  /* a94a8fe5ccb19ba61c4c0873d391e987982fbbd3  test.txt */
  hashResult = output.left(32).toLower();
#endif
  if (hashResult.isEmpty()) {
    LOG_W("Cannot parse output message: %s", qPrintable(output));
    return "";
  }
  return hashResult;
}

}
