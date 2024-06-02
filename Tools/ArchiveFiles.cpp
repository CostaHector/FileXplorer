#include "ArchiveFiles.h"
#include "FileOperation/FileOperatorPub.h"
#include "PublicVariable.h"
#include "UndoRedo.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>

constexpr int ArchiveFiles::PLAIN_TEXT_FILE_COMPRESS_LEVEL;

ArchiveFiles::ArchiveFiles(const QString& achieveName, const COMPRESS_FILETYPE_FILTER& compressFileType)
    : m_fi{achieveName}, m_compressFilesType{compressFileType} {}
ArchiveFiles::~ArchiveFiles() {
  if (m_fi.isOpen()) {
    m_fi.close();
  }
  m_ds.setDevice(nullptr);
}

bool ArchiveFiles::CompressADirectlyPath(const QString& path, const QString& qzBaseName, QString& allPres, QString& allNames) {
  static constexpr int MAX_COMPRESSED_IMG_CNT = 20;
  QDir qzPrepathDir{path, "", QDir::SortFlag::Name, QDir::Filter::Files};
  qzPrepathDir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  const QStringList& imgNames = qzPrepathDir.entryList();
  if (imgNames.isEmpty()) {
    return false;
  }
  if (imgNames.size() > MAX_COMPRESSED_IMG_CNT) {
    qDebug("Images counts[%d] > %d, cannot compress", imgNames.size(), MAX_COMPRESSED_IMG_CNT);
    return false;
  }
  QString pres, names;

  QStringList imgPaths;
  imgPaths.reserve(imgNames.size());
  for (const QString& imgName : imgNames) {
    imgPaths.append(qzPrepathDir.absoluteFilePath(imgName));
    pres += (path + '\n');
    names += (imgName + '\n');
  }
  const QString& qzFilePath = qzPrepathDir.absoluteFilePath(qzBaseName + ".qz");
  ArchiveFiles af{qzFilePath};
  bool compressedRet = af.CompressNow(OPERATION_TYPE::FILES, imgPaths, false);
  if (compressedRet) {
    allPres += (pres + '\n');
    allNames += (names + '\n');
  }
  return compressedRet;
}

int ArchiveFiles::CompressImagesByGroup(const QString& rootPath, bool recycleAfterCompressed) {
  QDir rootDir{rootPath, "", QDir::SortFlag::Name, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot};

  int compressedCnt = 0;
  int folderTotalCnt = 0;

  QString allPres, allNames;
  for (const QString& sub : rootDir.entryList()) {
    const QString& directPath = rootDir.absoluteFilePath(sub);
    compressedCnt += CompressADirectlyPath(directPath, sub, allPres, allNames);
    ++folderTotalCnt;
  }
  const QString& rootQZBaseName = QFileInfo(rootPath).completeBaseName();
  compressedCnt += CompressADirectlyPath(rootPath, rootQZBaseName, allPres, allNames);
  ++folderTotalCnt;
  qWarning("compressed %d/%d folders", compressedCnt, folderTotalCnt);

  if (recycleAfterCompressed and compressedCnt > 0) {
    FileOperatorType::BATCH_COMMAND_LIST_TYPE recycleCmds{{"moveToTrash", allPres, allNames}};
    bool recycleRet = g_undoRedo.Do(recycleCmds);
    if (recycleRet) {
      qDebug("Recycle succeed. %d files", recycleCmds.size());
    } else {
      qWarning("Some recycle failed. %d files", recycleCmds.size());
    }
  }
  return compressedCnt;
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
    qWarning("Cannot compress multi [%u]folders", paths.size());
    return false;
  }

  const QString folderPath = paths.front();
  QDirIterator srcDirIt{folderPath, GetCompressType(), QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  const int PREPATH_LEN = folderPath.size() + 1;  // "C:/home/file.txt", prelen = "C:/home".size() + 1
  int compressedFilesCnt = 0;
  while (srcDirIt.hasNext()) {
    srcDirIt.next();

    QFile srcFi{srcDirIt.filePath()};
    if (not srcFi.open(QFile::OpenModeFlag::ReadOnly)) {
      qWarning("file[%s] to compress cannot open to read.", qPrintable(srcFi.fileName()));
      return false;
    }
    ++compressedFilesCnt;
    QByteArray data = compress(srcFi.readAll());
    m_ds << srcFi.fileName().mid(PREPATH_LEN) << data;
    qDebug("compress file[%s] (%d bytes) to achieve ok", qPrintable(srcFi.fileName().mid(PREPATH_LEN)), data.size());
  }
  qDebug("compress folder ok: append %d file(s)", compressedFilesCnt);
  return true;
}

bool ArchiveFiles::AppendFiles(const QStringList& filesPath) {
  for (const QString& filePath : filesPath) {
    QFileInfo fi{filePath};
    if (not IsNeedCompress(fi.completeSuffix())) {
      continue;
    }
    QFile srcFi{filePath};
    if (not srcFi.open(QFile::OpenModeFlag::ReadOnly)) {
      qWarning("Cannot open file[%s]. (not exist or not file)", qPrintable(srcFi.fileName()));
      m_ds.setDevice(nullptr);
    }
    QByteArray data = compress(srcFi.readAll());
    m_ds << fi.fileName() << data;
    qDebug("Compress file[%s] (%d bytes) to achieve ok", qPrintable(QFileInfo(filePath).fileName()), data.size());
  }
  qDebug("Compress files ok: append %u file(s)", filesPath.size());
  return true;
}

QVariantList ArchiveFiles::PreviewFirstKItems(int k) {
  if (not m_fi.exists()) {
    qWarning("achieve file[%s] not exist", qPrintable(m_fi.fileName()));
    return {};
  }
  if (not m_fi.fileName().toLower().endsWith(".qz")) {
    qWarning("achieve file[%s] is not builtin compressed file type", qPrintable(m_fi.fileName()));
    return {};
  }
  if (not m_fi.isOpen()) {
    if (not m_fi.open(QFile::OpenModeFlag::ReadOnly)) {
      return {};
    }
  }
  if (m_ds.device() == nullptr) {
    m_ds.setDevice(&m_fi);
  }
  QVariantList itemsList;
  while (not m_ds.atEnd()) {
    QString relFilePath;
    QByteArray fileData;
    m_ds >> relFilePath >> fileData;

    itemsList.append(decompress(fileData));
    if (k != -1 and itemsList.size() >= k) {
      break;
    }
  }
  m_ds.setDevice(nullptr);
  m_fi.close();
  return itemsList;
}

bool ArchiveFiles::ReadFirstKItemsOut(int k, QStringList& paths, QByteArrayList& datas) {
  // -1 get all
  paths.clear();
  datas.clear();
  if (not m_fi.exists()) {
    qWarning("achieve file[%s] not exist", qPrintable(m_fi.fileName()));
    return false;
  }
  if (not m_fi.fileName().toLower().endsWith(".qz")) {
    qWarning("achieve file[%s] is not builtin compressed file type", qPrintable(m_fi.fileName()));
    return false;
  }
  if (not m_fi.isOpen()) {
    if (not m_fi.open(QFile::OpenModeFlag::ReadOnly)) {
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
    paths.append(relFilePath);
    datas.append(decompress(fileData));
    if (k != -1 and paths.size() >= k) {
      break;
    }
  }
  m_ds.setDevice(nullptr);
  m_fi.close();
  return true;
}

bool ArchiveFiles::DecompressToPath(const QString& dstPath) {
  QStringList paths;
  QList<QByteArray> datas;
  if (not ReadFirstKItemsOut(-1, paths, datas)) {
    qWarning("Read all files out failed");
    return false;
  }

  QDir dstDir{dstPath};
  if (not dstDir.exists()) {
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
  for (int i = 0; i < paths.size(); ++i) {
    const QString& relFilePath = paths[i];
    const QByteArray& fileData = datas[i];
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
