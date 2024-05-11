#include "ArchieveFiles.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>

constexpr int ArchieveFiles::PLAIN_TEXT_FILE_COMPRESS_LEVEL;

ArchieveFiles::ArchieveFiles(const QString& achieveName) : m_achievePath{achieveName} {}

bool ArchieveFiles::appendAFolder(const QString& folderPath) {
  if (m_fi.fileName().isEmpty()) {
    m_fi.setFileName(m_achievePath);
  }
  if (not m_fi.isOpen()) {
    bool openRes = m_fi.open(QFile::OpenModeFlag::WriteOnly | QFile::OpenModeFlag::Append);
    if (not openRes) {
      qWarning("Open achieve file[%s] to write into failed", qPrintable(m_fi.fileName()));
      return false;
    }
  }
  if (m_ds.device() == nullptr) {
    m_ds.setDevice(&m_fi);
  }

  QDirIterator srcDirIt{folderPath, {}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  const int PREPATH_LEN = folderPath.size() + 1;  // "C:/home/file.txt", prelen = "C:/home".size() + 1
  while (srcDirIt.hasNext()) {
    srcDirIt.next();

    QFile srcFi{srcDirIt.filePath()};
    if (not srcFi.open(QFile::OpenModeFlag::ReadOnly)) {
      qWarning("file[%s] to compress cannot open to read.", qPrintable(srcFi.fileName()));
      m_ds.setDevice(nullptr);
      m_fi.close();
      return false;
    }
    QByteArray data = compress(srcFi.readAll());
    m_ds << srcFi.fileName().mid(PREPATH_LEN) << data;
    qDebug("compress file[%s] (%d bytes) to achieve ok", qPrintable(srcFi.fileName().mid(PREPATH_LEN)), data.size());
  }

  m_ds.setDevice(nullptr);
  m_fi.close();
  return true;
}

bool ArchieveFiles::appendAFile(const QString& filePath) {
  if (m_fi.fileName().isEmpty()) {
    m_fi.setFileName(m_achievePath);
  }
  if (not m_fi.isOpen()) {
    bool openRes = m_fi.open(QFile::OpenModeFlag::WriteOnly | QFile::OpenModeFlag::Append);
    if (not openRes) {
      qWarning("Open achieve file[%s] to write into failed", qPrintable(m_fi.fileName()));
      return false;
    }
  }
  if (m_ds.device() == nullptr) {
    m_ds.setDevice(&m_fi);
  }

  QFile srcFi{filePath};
  if (not srcFi.open(QFile::OpenModeFlag::ReadOnly)) {
    qWarning("file[%s] to compress cannot open to read.", qPrintable(srcFi.fileName()));
    m_ds.setDevice(nullptr);
    m_fi.close();
    return false;
  }
  QByteArray data = compress(srcFi.readAll());
  m_ds << QFileInfo(filePath).fileName() << data;
  qDebug("compress file[%s] (%d bytes) to achieve ok", qPrintable(QFileInfo(filePath).fileName()), data.size());
  m_ds.setDevice(nullptr);
  m_fi.close();
  return true;
}

bool ArchieveFiles::appendFiles(const QStringList& filesPath) {
  if (m_fi.fileName().isEmpty()) {
    m_fi.setFileName(m_achievePath);
  }
  if (not m_fi.isOpen()) {
    bool openRes = m_fi.open(QFile::OpenModeFlag::WriteOnly | QFile::OpenModeFlag::Append);
    if (not openRes) {
      qWarning("Open achieve file[%s] to write into failed", qPrintable(m_fi.fileName()));
      return false;
    }
  }
  if (m_ds.device() == nullptr) {
    m_ds.setDevice(&m_fi);
  }
  for (const QString& filePath : filesPath) {
    QFile srcFi{filePath};
    if (not srcFi.open(QFile::OpenModeFlag::ReadOnly)) {
      qWarning("file[%s] to compress cannot open to read.", qPrintable(srcFi.fileName()));
      m_ds.setDevice(nullptr);
      m_fi.close();
      return false;
    }
    QByteArray data = compress(srcFi.readAll());
    m_ds << QFileInfo(filePath).fileName() << data;
    qDebug("compress file[%s] (%d bytes) to achieve ok", qPrintable(QFileInfo(filePath).fileName()), data.size());
  }
  m_ds.setDevice(nullptr);
  m_fi.close();
  return true;
}

bool ArchieveFiles::decompressToPath(const QString& dstPath) {
  QDir dstDir{dstPath};
  if (not dstDir.exists()) {
    return false;
  }
  if (not m_fi.isOpen()) {
    bool openRes = m_fi.open(QFile::OpenModeFlag::ReadOnly);
    if (not openRes) {
      return false;
    }
  }
  if (m_ds.device() == nullptr) {
    m_ds.setDevice(&m_fi);
  }

  while (not m_ds.atEnd()) {
    QString relFilePath;
    QByteArray fileData;
    m_ds >> relFilePath >> fileData;

    const QString& absDstPath = dstDir.absoluteFilePath(relFilePath);
    QFileInfo dstFi{absDstPath};
    if (not dstFi.absoluteDir().exists()) {
      if (not QDir().mkpath(dstFi.absolutePath())) {
        qWarning("cannot mkpath[%s] to place file[%s]", qPrintable(dstFi.absolutePath()), qPrintable(relFilePath));
        return false;
      }
    }
    fileData = decompress(fileData);
    QFile dstFile{absDstPath};
    if (not dstFile.open(QFile::OpenModeFlag::WriteOnly)) {
      qWarning("decompress [%d] bytes to file[%s] failed", fileData.size(), qPrintable(relFilePath));
      return false;
    }
    qDebug("decompress [%d] bytes to file[%s] ok", fileData.size(), qPrintable(relFilePath));
    dstFile.write(fileData);
    dstFile.close();
  }

  m_ds.setDevice(nullptr);
  m_fi.close();
  return true;
}

//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
int main(int argc, char* argv[]) {
  ArchieveFiles af{"E:/py/1/achieve.ach"};
  af.deleteAchieveFile();
  if (af.appendAFolder("E:/py/1/nostructure")) {
    af.decompressToPath("E:/py/1/dst");
  }
  return 0;
}
#endif
