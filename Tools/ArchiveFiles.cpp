#include "ArchiveFiles.h"
#include "FileOperatorPub.h"
#include "PublicVariable.h"
#include "UndoRedo.h"
#include "PathTool.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>

constexpr int ArchiveFilesReader::INFO_ITEMS_CNT;

bool ArchiveFilesReader::isQZFile(const QString& path) {
  return TYPE_FILTER::BUILTIN_COMPRESSED_TYPE_SET.contains(PathTool::GetAsteriskDotFileExtension(path));
}

bool ArchiveFilesReader::isQZFile(const QFileInfo& fi) {
  return isQZFile(fi.fileName());
}

bool ArchiveFilesReader::ReadAchiveFile(const QString& qzFilePath) {
  clear();

  if (!isQZFile(qzFilePath)) {
    LOG_D("File[%s] not a qz file", qPrintable(qzFilePath));
    return false;
  }
  QFileInfo fileInfo(qzFilePath);
  if (!fileInfo.exists() || !fileInfo.isFile()) {
    LOG_D("File[%s] not exist", qPrintable(qzFilePath));
    return false;
  }

  m_fi.setFileName(qzFilePath);
  if (!m_fi.isOpen()) {
    if (!m_fi.open(QFile::OpenModeFlag::ReadOnly)) {
      LOG_W("Open file[%s] for read failed", qPrintable(qzFilePath));
      return false;
    }
  }

  m_ds.setDevice(&m_fi);
  m_ds.resetStatus();
  int itemsCount = 0;
  while (!m_ds.atEnd()) {
    if (!ReadAnItem()) {
      LOG_W("Failed to read the %dth item", itemsCount + 1);
      clear();
      return false;
    }
    ++itemsCount;
  }
  if (m_ds.status() != QDataStream::Ok) {
    LOG_W("Error reading archive: stream status %d", m_ds.status());
    clear();
    return false;
  }

  LOG_D("%d items successfully read from[%s]", itemsCount, qPrintable(qzFilePath));
  return true;
}

bool ArchiveFilesReader::ReadAnItem() {
  QString relFilePath;
  qint64 beforeSize = 0;
  int afterSize = 0;
  QByteArray fileData;
  if (INFO_ITEMS_CNT == 4) {
    m_ds >> relFilePath >> beforeSize >> afterSize >> fileData;
  } else if (INFO_ITEMS_CNT == 2) {
    m_ds >> relFilePath >> fileData;
  }
  if (m_ds.status() != QDataStream::Ok) {
    LOG_W("Failed to read item data. Stream status: %d", m_ds.status());
    return false;
  }
  m_items.push_back(CompressedItem{relFilePath, beforeSize, afterSize, fileData});
  return true;
}

bool ArchiveFilesReader::DecompressToPath(const QString& dstPath) const {
  QDir dstDir{dstPath};
  if (!dstDir.exists()) {
    LOG_W("destination path[%s] not exist", qPrintable(dstPath));
    return false;
  }

  for (int i = 0; i < size(); ++i) {
    const QString& relFilePath = key(i);
    const QString& destAbsFilePath = dstDir.absoluteFilePath(relFilePath);
    if (QFile::exists(destAbsFilePath)) {
      LOG_W("File[%s] already exist, skip overwrite", qPrintable(destAbsFilePath));
      continue;
    }

    QString prePath;
    QString fileName = PathTool::GetPrepathAndFileName(destAbsFilePath, prePath);
    if (!QFileInfo(prePath).isDir()) {
      if (QDir().mkpath(prePath)) {
        LOG_W("Cannot mkpath[%s]", qPrintable(prePath));
        continue;
      }
    }

    QFile dstFile{destAbsFilePath};
    if (!dstFile.open(QFile::OpenModeFlag::WriteOnly)) {
      LOG_W("Open file[%s] to write byteData failed", qPrintable(relFilePath));
      return false;
    }
    const QByteArray& fileData = value(i);
    const qint64 succeedBytesCount = dstFile.write(fileData);
    LOG_OE(succeedBytesCount >= 0, "Decompress[%d] bytes to file[%s] ok", fileData.size(), qPrintable(relFilePath));

    dstFile.close();
  }
  return true;
}

bool ArchiveFilesWriter::CompressNow(const QString& workPath, const QStringList& fileNames, const QString& destArchiveFileName, bool enableAppend) {
  if (fileNames.isEmpty()) {
    LOG_D("no files need to compress");
    return true;
  }
  QFile::OpenMode openMde = QFile::OpenModeFlag::WriteOnly;
  if (enableAppend) {
    openMde.setFlag(QFile::OpenModeFlag::Append);  // append
  } else {
    openMde.setFlag(QFile::OpenModeFlag::Truncate);  // overwrite
  }

  m_fi.close();
  m_fi.setFileName(destArchiveFileName);
  if (!m_fi.open(openMde)) {
    LOG_W("Cannot open achive file[%s] to write into.", qPrintable(destArchiveFileName));
    return false;
  }
  m_ds.setDevice(&m_fi);
  m_ds.resetStatus();

  auto cleanupAndReturn = [this](bool returnValue) -> bool {
    m_fi.close();
    m_ds.setDevice(nullptr);
    return returnValue;
  };

  int compressItemsCount = 0;
  QDir workDir{workPath};
  for (const QString& name : fileNames) {
    QFileInfo fileNeedToCompressFi{workDir, name};
    if (!fileNeedToCompressFi.exists()) {
      LOG_D("File[%s] under %s not exist at all", qPrintable(name), qPrintable(workPath));
      continue;
    }
    if (fileNeedToCompressFi.isFile()) {
      int result = AddOneFile(workPath, name);
      if (!result) {
        return cleanupAndReturn(false);
      }
      continue;
    }
    if (fileNeedToCompressFi.isDir()) {
      int cnt = AddOneFolder(workPath, name);
      if (cnt < 0) {
        return cleanupAndReturn(false);
      }
      compressItemsCount += cnt;
      continue;
    }
  }
  LOG_D("%d item(s) compressed ok [from %d selected under %s]", compressItemsCount, fileNames.size(), qPrintable(workPath));
  return cleanupAndReturn(true);
}

bool ArchiveFilesWriter::IsNeedCompress(const QFile& fi) {
  if (fi.size() > 10 * 1024 * 1024) {
    return false;
  }
  const QString asteriskDotExt = PathTool::GetAsteriskDotFileExtension(fi.fileName());
  if (TYPE_FILTER::VIDEO_TYPE_SET.contains(asteriskDotExt)) {
    return false;
  }
  return true;
}

int ArchiveFilesWriter::AddOneFolder(const QString& workPath, const QString& rel2FolderName) {
  if (m_ds.device() == nullptr) {
    return -1;
  }
  const int PREPATH_LEN = workPath.size() + 1;  // "C:/home/file.txt", prelen = "C:/home".size() + 1
  const QString absFolderPath = workPath + '/' + rel2FolderName;
  int compSuccCnt = 0;
  QDirIterator srcDirIt(absFolderPath, {}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (srcDirIt.hasNext()) {
    const QString fileToBeCompressed = srcDirIt.next();
    QFile fi{fileToBeCompressed};
    if (!IsNeedCompress(fi)) {
      continue;
    }
    if (!fi.open(QFile::OpenModeFlag::ReadOnly)) {
      LOG_W("File[%s] not exist or not a file, cannot compressed", qPrintable(fi.fileName()));
      return -1;
    }
    QByteArray data = fi.readAll();
    WriteIntoFile(fileToBeCompressed.mid(PREPATH_LEN), fi.size(), data);
    if (m_ds.status() != QDataStream::Ok) {
      LOG_W("File[%s] write into qz file failed", qPrintable(fileToBeCompressed));
      return -1;
    }
    ++compSuccCnt;
  }
  LOG_D("%d files under[%s] compressed ok", compSuccCnt, qPrintable(absFolderPath));
  return compSuccCnt;
}

bool ArchiveFilesWriter::AddOneFile(const QString& workPath, const QString& rel2FileName) {
  if (m_ds.device() == nullptr) {
    return -1;
  }
  QFile srcFi{workPath + '/' + rel2FileName};
  if (!IsNeedCompress(srcFi)) {
    LOG_D("skip file[%s]", qPrintable(srcFi.fileName()));
    return true;
  }
  if (!srcFi.open(QFile::OpenModeFlag::ReadOnly)) {
    LOG_W("File[%s] not exist or not a file, cannot compressed", qPrintable(srcFi.fileName()));
    return false;
  }
  const QByteArray& srcData = srcFi.readAll();
  WriteIntoFile(rel2FileName, srcFi.size(), srcData);
  QDataStream::Status dataStreamCode = m_ds.status();
  if (dataStreamCode != QDataStream::Ok) {
    LOG_W("File[%s] write into qz file failed", qPrintable(srcFi.fileName()));
    return false;
  }
  return true;
}

constexpr int ArchiveImagesRecusive::MAX_COMPRESSED_IMG_CNT;
int ArchiveImagesRecusive::CompressImgRecur(const QString& rootPath) {
  QDir rootDir{rootPath, "", QDir::SortFlag::Name, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot};
  QStringList paths{rootDir.entryList()};  // all direct subfolders only
  paths.append("");                        // root path itself

  int folderSuccCnt = 0;
  int folderTotalCnt = 0;

  for (const QString& sub : paths) {
    const QString& archiveName = sub.isEmpty() ? QFileInfo(rootPath).completeBaseName() : sub;
    const QString& directPath = rootDir.absoluteFilePath(sub);
    folderSuccCnt += CompressSubfolder(directPath, archiveName);
    ++folderTotalCnt;
  }
  LOG_W("compressed %d/%d folders ok", folderSuccCnt, folderTotalCnt);

  using namespace FileOperatorType;
  if (m_autoRecycle && folderSuccCnt > 0) {
    BATCH_COMMAND_LIST_TYPE recycleCmds;
    recycleCmds.reserve(m_allPres.size());
    for (int i = 0; i < m_allPres.size(); ++i) {
      recycleCmds.append(ACMD::GetInstMOVETOTRASH(m_allPres[i], m_allNames[i]));
    }

    bool recycleRet = UndoRedo::GetInst().Do(recycleCmds);
    LOG_OE(recycleRet, "Recyle %d files", recycleCmds.size());
  }
  return folderSuccCnt;
}

bool ArchiveImagesRecusive::CompressSubfolder(const QString& path, const QString& qzBaseName) {
  QDir qzPrepathDir{path, "", QDir::SortFlag::Name, QDir::Filter::Files};
  qzPrepathDir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  const QStringList& imgNames = qzPrepathDir.entryList();
  if (imgNames.isEmpty()) {
    return true;
  }
  if (imgNames.size() > MAX_COMPRESSED_IMG_CNT) {
    LOG_W("Images in folder[%s] counts[%d] > %d. Reject compress", qPrintable(path), imgNames.size(), MAX_COMPRESSED_IMG_CNT);
    return false;
  }

  const QString& qzFilePath = qzPrepathDir.absoluteFilePath(qzBaseName + ".qz");

  ArchiveFilesWriter af;
  bool compRet = af.CompressNow(path, imgNames, qzFilePath, false);
  if (m_autoRecycle && !compRet) {
    for (int i = 0; i < imgNames.size(); ++i) {
      m_allPres.append(path);
    }
    m_allNames += imgNames;
  }
  return compRet;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
int main(int argc, char* argv[]) {
  ArchiveFiles af{"E:/py/1/achieve.qz"};

  af.CompressDelegate(ArchiveFilesWriter::FILES, {"E:/py/1/testImage.jpg"}, false);
  if (af.CompressDelegate(ArchiveFilesWriter::A_FOLDER, {"E:/py/1/nostructure"}, true)) {
    af.DecompressToPath("E:/py/1/dst");
  }
  return 0;
}
#endif
