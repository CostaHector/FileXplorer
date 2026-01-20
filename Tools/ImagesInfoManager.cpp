#include "ImagesInfoManager.h"
#include "MD5Calculator.h"
#include "PublicVariable.h"
#include "PathTool.h"
#include "MemoryKey.h"
#include "NotificatorMacro.h"
#include <QDirIterator>

template class SingletonManager<ImagesInfoManager, IMG_INFO_DATA_T>;

using namespace DuplicateImageMetaInfo;

QString ImagesInfoManager::GetDynRedunPath() const {
  return Configuration().value(RedunImgFinderKey::RUND_IMG_PATH.name, RedunImgFinderKey::RUND_IMG_PATH.v).toString();
}

ImagesInfoManager::ImagesInfoManager() {
#ifndef RUNNING_UNIT_TESTS
  const QString benchMarkPath = GetDynRedunPath();
  InitializeImpl(benchMarkPath);
#endif
}

void ImagesInfoManager::InitializeImpl(const QString& path) {
  // don't save path to member here. path existence is not guaranteed
  Configuration().setValue(RedunImgFinderKey::RUND_IMG_PATH.name, path);
  ImgDataStruct() = ReadOutImgsInfo();
}

IMG_INFO_DATA_T ImagesInfoManager::ReadOutImgsInfo() const {
  const QString benchmarkPath = GetDynRedunPath();
  if (!QFileInfo{benchmarkPath}.isDir()) {
    LOG_W("Benchmark path[%s] not exists", qPrintable(benchmarkPath));
    return {};
  }
  IMG_INFO_DATA_T imgMgrData;
  decltype(imgMgrData.m_commonFileHash) hashSet;
  decltype(imgMgrData.m_commonFileSizeSet) sizeSet;

  int filesCnt = 0;
  QDirIterator it{benchmarkPath, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    const QString fileAbsPath = it.next();
    const qint64 sz = QFile{fileAbsPath}.size();
    const QByteArray& md5 = MD5Calculator::GetFileMD5(fileAbsPath);
    sizeSet.insert(sz);
    hashSet.insert(md5);
    ++filesCnt;
  }
  LOG_D("benchmarkPath[%s] sizeSet count[%d] and hashSet count[%d] from %d file(s)", //
        qPrintable(benchmarkPath),
        sizeSet.size(),
        hashSet.size(),
        filesCnt);
  imgMgrData.m_commonFileHash.swap(hashSet);
  imgMgrData.m_commonFileSizeSet.swap(sizeSet);
  return imgMgrData;
}

int ImagesInfoManager::ForceReloadImpl() {
  auto& before = ImgDataStruct();
  int beforeHashCnt = before.size();
  int beforeSzCnt = before.size();

  auto after = ReadOutImgsInfo();
  int afterHashCnt = after.size();
  int afterSzCnt = after.size();
  before.swap(after);

  const QString benchmarkPath = GetDynRedunPath();
  LOG_OK_P(benchmarkPath,
           "sizeSet count[%d->%d] and hashSet count delta[%d->%d]", //
           beforeSzCnt,
           afterSzCnt,
           beforeHashCnt,
           afterHashCnt);
  return afterSzCnt + afterHashCnt - beforeSzCnt - beforeHashCnt;
}

RedundantImagesList ImagesInfoManager::FindRedunImgs(const QString& folderPath, const bool bAlsoFindEmpty) const {
  QDirIterator it{folderPath, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  RedundantImagesList redundantImgs;
  while (it.hasNext()) {
    const QFileInfo imgFi{it.next()};
    const QString fileAbsPath = imgFi.absoluteFilePath();
    const qint64 sz = imgFi.size();
    if (sz == 0) {
      if (bAlsoFindEmpty) {
        redundantImgs.append(REDUNDANT_IMG_INFO{imgFi.fileName(), 0, "", fileAbsPath});
      }
      continue;
    }
    if (!ImgDataStruct().contains(sz)) {
      continue;
    }
    const QByteArray& md5 = MD5Calculator::GetFileMD5(fileAbsPath);
    if (!ImgDataStruct().contains(md5)) {
      continue;
    }
    redundantImgs.append(REDUNDANT_IMG_INFO{imgFi.fileName(), sz, md5, fileAbsPath});
  }
  return redundantImgs;
}

#ifdef RUNNING_UNIT_TESTS
int ImagesInfoManager::ResetStateForTestImpl(const QString& localFilePath) {
  InitializeImpl(localFilePath);
  return 0;
}
#endif

RedundantImagesList FindDuplicateImgs(const QString& folderPath, const bool bAlsoFindEmpty) {
  QDirIterator it{folderPath, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  QMap<qint64, QStringList> sz2AbsPath;
  while (it.hasNext()) {
    const QFileInfo imgFi{it.next()};
    sz2AbsPath[imgFi.size()].push_back(imgFi.absoluteFilePath());
  }

  RedundantImagesList dupImgs;
  for (auto it = sz2AbsPath.cbegin(); it != sz2AbsPath.cend(); ++it) {
    if (it.key() == 0) {
      if (bAlsoFindEmpty) {
        for (const auto& absPath : it.value()) {
          dupImgs.append(REDUNDANT_IMG_INFO{PathTool::fileName(absPath), 0, "", absPath});
        }
      }
      continue;
    }
    if (it.value().size() < 2) {
      continue;
    }
    QMap<QByteArray, QString> hash2FirstDuplicateFile;
    QSet<QByteArray> firstDuplicateFileMd5;
    for (const auto& absPath : it.value()) {
      const QByteArray& md5 = MD5Calculator::GetFileMD5(absPath);
      auto sameMd5It = hash2FirstDuplicateFile.find(md5);
      if (sameMd5It != hash2FirstDuplicateFile.cend()) {
        if (firstDuplicateFileMd5.find(md5) == firstDuplicateFileMd5.end()) {
          QString fileAbsPath = sameMd5It.value();
          dupImgs.append(REDUNDANT_IMG_INFO{PathTool::fileName(fileAbsPath), QFile{fileAbsPath}.size(), md5, fileAbsPath});
          firstDuplicateFileMd5.insert(md5);
        }
        dupImgs.append(REDUNDANT_IMG_INFO{PathTool::fileName(absPath), QFile{absPath}.size(), md5, absPath});
      } else {
        hash2FirstDuplicateFile[md5] = absPath;
      }
    }
  }
  return dupImgs;
}
