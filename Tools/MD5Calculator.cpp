#include "MD5Calculator.h"
#include "Logger.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QFileInfo>

namespace MD5Calculator {

QByteArray GetFileMD5(const QString& filepath, const BytesRangeTool::BytesRangeE firstBytesRangeInt, QCryptographicHash::Algorithm alg) {
  QFile file{filepath};
  if (!file.exists()) {
    LOG_W("file[%s] not found", qPrintable(filepath));
    return "";
  }
  if (!file.open(QIODevice::ReadOnly)) {
    LOG_D("file[%s] open failed", qPrintable(filepath));
    return "";
  }
  const qint64 sz = file.size();
  const int bytesShouldRead = BytesRangeTool::toBytesValue(firstBytesRangeInt);
  if (firstBytesRangeInt != BytesRangeTool::BytesRangeE::ENTIRE_FILE) {
    switch (firstBytesRangeInt) {
      case BytesRangeTool::BytesRangeE::FIRST_16_BYTES:
        return GetByteArrayMD5(file.read(16), alg);
      case BytesRangeTool::BytesRangeE::FIRST_1_KB:
        return GetByteArrayMD5(file.read(1024), alg);
      case BytesRangeTool::BytesRangeE::SAMPLED_128_KB:
      case BytesRangeTool::BytesRangeE::SAMPLED_512_KB: {
        if (sz < bytesShouldRead) {
          break;
        }
        static constexpr int SAMPLE_TIME = 8;
        const int blockSizeEachTimeRead = bytesShouldRead / SAMPLE_TIME;
        const qint64 interval = sz / SAMPLE_TIME;
        QCryptographicHash md5(alg);
        int readTime = 0;
        do {
          md5.addData(file.read(blockSizeEachTimeRead));
          ++readTime;
        } while(readTime < SAMPLE_TIME && file.seek(readTime * interval));
        return md5.result().toHex();
      }
      case BytesRangeTool::BytesRangeE::ENTIRE_FILE:
      default:
        break;
    }
  }
  QCryptographicHash md5(alg);
  while (!file.atEnd()) {
    md5.addData(file.read(8 * 1024));
  }
  file.close();
  return md5.result().toHex();
}

QByteArray GetByteArrayMD5(const QByteArray& ba, QCryptographicHash::Algorithm alg) {
  QCryptographicHash md5(QCryptographicHash::Md5);
  md5.addData(ba);
  return md5.result().toHex();
}

QList<QByteArray> GetBatchFileMD5(const QStringList& filepaths,
                                  const BytesRangeTool::BytesRangeE firstBytesRangeInt,
                                  QCryptographicHash::Algorithm alg) {
  QList<QByteArray> md5Lst;
  md5Lst.reserve(filepaths.size());
  for (const auto& path : filepaths) {
    QFileInfo fi(path);
    if (!fi.exists() || !fi.isFile()) {
      continue;
    }
    md5Lst << GetFileMD5(path, firstBytesRangeInt, alg);
  }
  return md5Lst;
}

QString DisplayFilesMD5(const QStringList& fileAbsPaths) {
  return MD5PrepathName2Table(GetBatchFileMD5(fileAbsPaths), fileAbsPaths);
}

QString MD5PrepathName2Table(const QList<QByteArray>& md5s, const QStringList& fileAbsPaths) {
  QStringList fileNames, fileDirs;
  for (const auto& pth : fileAbsPaths) {
    const QFileInfo fi{pth};
    if (!fi.exists() || !fi.isFile()) {
      continue;
    }
    fileNames << fi.fileName();
    fileDirs << fi.absolutePath();
  }
  QString nameHashListStr;
  nameHashListStr += QString::asprintf("MD5 of %d file(s)\n", md5s.size());
  nameHashListStr += MD5DetailHtmlTable(md5s, fileNames, fileDirs);
  return nameHashListStr;
}

QString MD5DetailHtmlTable(const QList<QByteArray>& md5s, const QStringList& fileNames, const QStringList& fileDirs) {
  if (!(md5s.size() == fileNames.size() && fileNames.size() == fileDirs.size())) {
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
    rows += MD5_TABLE_ROW_TEMPLATE.arg(QString::fromUtf8(md5s[i])).arg(fileNames[i]).arg(fileDirs[i]);
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

}  // namespace MD5Calculator
