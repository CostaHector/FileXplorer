#include "ArchiveFiles.h"
#include "FileOperation/FileOperatorPub.h"
#include "PublicVariable.h"
#include "UndoRedo.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>

constexpr int ArchiveFiles::PLAIN_TEXT_FILE_COMPRESS_LEVEL;
constexpr int ArchiveFiles::MAX_COMPRESSED_IMG_CNT;
constexpr int ArchiveFiles::INFO_ITEMS_CNT;

ArchiveFiles::ArchiveFiles(const QString& achieveName, const COMPRESS_FILETYPE_FILTER& compressFileType)
    : m_fi{achieveName}, m_compressFilesType{compressFileType} {
  ReadItemsCount();
}

void ArchiveFiles::swap(ArchiveFiles& rhs) {
  QString beforeName = m_fi.fileName(), afterName = rhs.m_fi.fileName();
  beforeName.swap(afterName);
  m_fi.setFileName(beforeName);
  rhs.m_fi.setFileName(afterName);
  // --
  std::swap(m_compressFilesType, rhs.m_compressFilesType);
  // --
  auto *beforeDev = m_ds.device(), *afterDev = rhs.m_ds.device();
  std::swap(beforeDev, afterDev);
  m_ds.setDevice(beforeDev);
  rhs.m_ds.setDevice(afterDev);
  // --
  m_names.swap(rhs.m_names);
  m_beforeSize.swap(rhs.m_beforeSize);
  m_afterSize.swap(rhs.m_afterSize);
  m_datas.swap(rhs.m_datas);
}

ArchiveFiles::~ArchiveFiles() {
  if (m_fi.isOpen()) {
    m_fi.close();
  }
  m_ds.setDevice(nullptr);
}

bool ArchiveFiles::ReadItemsCount() {
  // files not exist is ok when read.
  if (not m_fi.exists()) {
    return false;
  }

  if (not isQZFile(m_fi.fileName())) {
    qDebug("File[%s]'s is not *.qz", qPrintable(m_fi.fileName()));
    return false;
  }

  if ((not m_fi.isOpen()) and (not m_fi.open(QFile::OpenModeFlag::ReadOnly))) {
    return false;
  }
  if (m_ds.device() == nullptr) {
    m_ds.setDevice(&m_fi);
  }
  while (not m_ds.atEnd()) {
    Read();
  }
  return true;
}
bool ArchiveFiles::isQZFile(const QFileInfo& fi) {
  return TYPE_FILTER::BUILTIN_COMPRESSED_TYPE_SET.contains("*." + fi.suffix());
}
bool ArchiveFiles::isQZFile(const QString& path) {
  return ArchiveFiles::isQZFile(QFileInfo(path));
}

bool ArchiveFiles::CompressNow(OPERATION_TYPE type, const QStringList& paths, bool enableAppend) {
  if (m_fi.fileName().isEmpty()) {
    qDebug("Achieve file name[%s] is invalid.", qPrintable(m_fi.fileName()));
    return false;
  }

  bool openRes = m_fi.open(QFile::OpenModeFlag::WriteOnly | (enableAppend ? QFile::OpenModeFlag::Append : QFile::OpenModeFlag::NotOpen));
  if (not openRes) {
    qWarning("Cannot open achieve file[%s] to write into.", qPrintable(m_fi.fileName()));
    return false;
  }

  if (m_ds.device() == nullptr) {
    m_ds.setDevice(&m_fi);
  }
  bool compressedResult = false;
  switch (type) {
    case A_FOLDER:
      compressedResult = AppendAFolder(paths);
      break;
    case FILES:
      compressedResult = AppendFiles(paths);
      break;
    default:
      break;
  }
  m_ds.setDevice(nullptr);
  m_fi.close();
  return compressedResult;
}

bool ArchiveFiles::IsNeedCompress(const QString& suffix) const {
  switch (m_compressFilesType) {
    case NO_FILTER:
      return true;
    case ONLY_IMAGE:
      return TYPE_FILTER::IMAGE_TYPE_SET.contains("*." + suffix);
    case ONLY_PLAIN_TEXT:
      return TYPE_FILTER::TEXT_TYPE_SET.contains("*." + suffix);
    default:
      return false;
  }
}
QStringList ArchiveFiles::GetCompressType() const {
  QStringList filesType;
  switch (m_compressFilesType) {
    case NO_FILTER:
      return QStringList{"*"};
    case ONLY_IMAGE:
      return TYPE_FILTER::IMAGE_TYPE_SET;
    case ONLY_PLAIN_TEXT:
      return TYPE_FILTER::TEXT_TYPE_SET;
    default:
      return QStringList{""};
  }
}

bool ArchiveFiles::AppendAFolder(const QStringList& paths) {
  if (paths.size() > 1) {
    qWarning("Cannot compress more than 1 folders now. folders count [%u]", paths.size());
    return false;
  }

  const QString& folderPath = paths.front();
  const int PREPATH_LEN = folderPath.size() + 1;  // "C:/home/file.txt", prelen = "C:/home".size() + 1

  int compSuccCnt = 0, compTotalCnt = 0;
  QDirIterator srcDirIt{folderPath, GetCompressType(), QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (srcDirIt.hasNext()) {
    srcDirIt.next();
    ++compTotalCnt;
    QFile fi{srcDirIt.filePath()};
    if (not fi.open(QFile::OpenModeFlag::ReadOnly)) {
      qWarning("File[%s] not exist or not a file, cannot compressed", qPrintable(fi.fileName()));
      continue;
    }
    QByteArray data = compress(fi.readAll());
    WriteIntoFile(fi.fileName().mid(PREPATH_LEN), fi.size(), data);
    ++compSuccCnt;
    qDebug("File[%s] (%d bytes) compressed ok", qPrintable(fi.fileName()), data.size());
  }
  qDebug("%d/%d files compressed ok", compSuccCnt, compTotalCnt);
  return compSuccCnt == compTotalCnt;
}

bool ArchiveFiles::AppendFiles(const QStringList& filesPath) {
  int compSuccCnt = 0, compTotalCnt = 0;
  for (const QString& filePath : filesPath) {
    QFileInfo fi{filePath};
    if (not IsNeedCompress(fi.completeSuffix())) {
      qDebug("skip file[%s]", qPrintable(filePath));
      continue;
    }
    ++compTotalCnt;
    QFile srcFi{filePath};
    if (not srcFi.open(QFile::OpenModeFlag::ReadOnly)) {
      qWarning("File[%s] not exist or not a file, cannot compressed", qPrintable(srcFi.fileName()));
      continue;
    }
    ++compSuccCnt;
    QByteArray data = compress(srcFi.readAll());
    WriteIntoFile(fi.fileName(), fi.size(), data);
    qDebug("File[%s] (%d bytes) compressed ok", qPrintable(fi.fileName()), data.size());
  }
  qDebug("%d/%d files compressed ok", compSuccCnt, compTotalCnt);
  return compSuccCnt == compTotalCnt;
}

bool ArchiveFiles::DecompressToPath(const QString& dstPath) {
  QDir dstDir{dstPath};
  if (!dstDir.exists()) {
    qWarning("destination path[%s] not exist", qPrintable(dstPath));
    return false;
  }

  auto makePrepath = [](const QString& pth) -> bool {
    QFileInfo dstFi{pth};
    if (dstFi.absoluteDir().exists())
      return true;
    return QDir().mkpath(dstFi.absolutePath());
  };

  QFile dstFile;
  for (int i = 0; i < size(); ++i) {
    const QString& relFilePath = m_names[i];
    const QByteArray& fileData = m_datas[i].toByteArray();
    const QString& absDstPath = dstDir.absoluteFilePath(relFilePath);
    if (not makePrepath(absDstPath)) {
      qWarning("Cannot parent folder of[%s]", qPrintable(absDstPath));
      return false;
    }

    dstFile.setFileName(absDstPath);
    if (not dstFile.open(QFile::OpenModeFlag::WriteOnly)) {
      qWarning("decompress [%d] bytes to file[%s] failed", fileData.size(), qPrintable(relFilePath));
      return false;
    }
    qDebug("decompress [%d] bytes to file[%s] ok", fileData.size(), qPrintable(relFilePath));
    dstFile.write(fileData);
    dstFile.close();
  }
  return true;
}

int ArchiveImagesRecusive::CompressImgRecur(const QString& rootPath) {
  QDir rootDir{rootPath, "", QDir::SortFlag::Name, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot};

  int folderSuccCnt = 0;
  int folderTotalCnt = 0;

  QStringList paths{rootDir.entryList()};  // all direct subfolders
  paths.append("");                        // root path itself

  for (const QString& sub : paths) {
    const QString& archiveName = sub.isEmpty() ? QFileInfo(rootPath).completeBaseName() : sub;
    const QString& directPath = rootDir.absoluteFilePath(sub);
    folderSuccCnt += CompressSubfolder(directPath, archiveName);
    ++folderTotalCnt;
  }
  qWarning("compressed %d/%d folders ok", folderSuccCnt, folderTotalCnt);
  using namespace FileOperatorType;
  if (m_autoRecycle and folderSuccCnt > 0) {
    BATCH_COMMAND_LIST_TYPE recycleCmds;
    recycleCmds.reserve(m_allPres.size());
    for (int i = 0; i < m_allPres.size(); ++i) {
      recycleCmds.append(ACMD{MOVETOTRASH, {m_allPres[i], m_allNames[i]}});
    }

    bool recycleRet = g_undoRedo.Do(recycleCmds);
    if (recycleRet) {
      qDebug("Recycle succeed. %d files", recycleCmds.size());
    } else {
      qWarning("Some recycle failed. %d files", recycleCmds.size());
    }
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
  if (imgNames.size() > ArchiveFiles::MAX_COMPRESSED_IMG_CNT) {
    qWarning("Images in folder[%s] counts[%d] > %d. Reject compress", qPrintable(path), imgNames.size(), ArchiveFiles::MAX_COMPRESSED_IMG_CNT);
    return false;
  }

  QStringList imgPaths;
  imgPaths.reserve(imgNames.size());
  for (const QString& imgName : imgNames) {
    imgPaths.append(qzPrepathDir.absoluteFilePath(imgName));
    if (not m_autoRecycle)
      continue;
    m_allPres.append(path);
    m_allNames.append(imgName);
  }

  const QString& qzFilePath = qzPrepathDir.absoluteFilePath(qzBaseName + ".qz");
  ArchiveFiles af{qzFilePath};
  bool compRet = af.CompressNow(ArchiveFiles::OPERATION_TYPE::FILES, imgPaths, false);
  if (m_autoRecycle and not compRet) {  // pop back when compressed failed
    m_allPres.erase(m_allPres.end() - imgNames.size(), m_allPres.end());
    m_allNames.erase(m_allNames.end() - imgNames.size(), m_allNames.end());
  }
  return compRet;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
int main(int argc, char* argv[]) {
  ArchiveFiles af{"E:/py/1/achieve.qz", ArchiveFiles::ONLY_IMAGE};
  af.deleteAchieveFile();

  af.CompressDelegate(ArchiveFiles::FILES, {"E:/py/1/testImage.jpg"}, false);
  if (af.CompressDelegate(ArchiveFiles::A_FOLDER, {"E:/py/1/nostructure"}, true)) {
    af.DecompressToPath("E:/py/1/dst");
  }
  return 0;
}
#endif
