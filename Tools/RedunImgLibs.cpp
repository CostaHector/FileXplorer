#include "RedunImgLibs.h"
#include "Tools/MD5Calculator.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include <QDirIterator>

QString RedunImgLibs::GetRedunPath() {
  QString redunPath;
#ifdef _WIN32
  redunPath = PreferenceSettings().value(RedunImgFinderKey::WIN32_RUND_IMG_PATH.name, RedunImgFinderKey::WIN32_RUND_IMG_PATH.v).toString();
#else
  redunPath = PreferenceSettings().value(RedunImgFinderKey::LINUX_RUND_IMG_PATH.name, RedunImgFinderKey::LINUX_RUND_IMG_PATH.v).toString();
#endif
  return redunPath;
}

int RedunImgLibs::LearnSizeAndHashFromRedunImgPath(const QString& folderPath) {
  qDebug("Benchmark redundant images located in [%s]", qPrintable(folderPath));
  int filesCnt = 0;
  QDirIterator it{folderPath, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    QFileInfo imgFi{it.next()};
    const QString fileAbsPath = imgFi.absoluteFilePath();
    const qint64 sz = imgFi.size();
    m_commonFileSizeSet.insert(sz);
    const QString& md5 = MD5Calculator::GetFileMD5(fileAbsPath);
    m_commonFileHash.insert(md5);
    ++filesCnt;
  }
  qDebug("redundant image info size[%d] and hash[%d]", m_commonFileSizeSet.size(), m_commonFileHash.size());
  return filesCnt;
}

REDUNDANT_IMG_BUNCH RedunImgLibs::FindRedunImgs(const QString& folderPath, const bool bAlsoFindEmpty) {
  QDirIterator it{folderPath, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  REDUNDANT_IMG_BUNCH redundantImgs;
  while (it.hasNext()) {
    const QFileInfo imgFi{it.next()};
    const QString fileAbsPath = imgFi.absoluteFilePath();
    const qint64 sz = imgFi.size();
    if (sz == 0) {
      if (bAlsoFindEmpty) {
        redundantImgs.append(REDUNDANT_IMG_INFO{fileAbsPath, 0, ""});
      }
      continue;
    }
    if (!m_commonFileSizeSet.contains(sz)) {
      continue;
    }
    const QString& md5 = MD5Calculator::GetFileMD5(fileAbsPath);
    if (!m_commonFileHash.contains(md5)) {
      continue;
    }
    redundantImgs.append(REDUNDANT_IMG_INFO{fileAbsPath, sz, md5});
  }
  return redundantImgs;
}

REDUNDANT_IMG_BUNCH RedunImgLibs::FindDuplicateImgs(const QString& folderPath, const bool bAlsoFindEmpty) {
  QDirIterator it{folderPath, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  QMap<qint64, QStringList> sz2AbsPath;
  while (it.hasNext()) {
    const QFileInfo imgFi{it.next()};
    sz2AbsPath[imgFi.size()].push_back(imgFi.absoluteFilePath());
  }

  REDUNDANT_IMG_BUNCH dupImgs;
  for (auto it = sz2AbsPath.cbegin(); it != sz2AbsPath.cend(); ++it) {
    if (it.key() == 0) {
      if (bAlsoFindEmpty) {
        for (const auto& absPath : it.value()) {
          dupImgs.append(REDUNDANT_IMG_INFO{absPath, 0, ""});
        }
      }
      continue;
    }
    if (it.value().size() < 2) {
      continue;
    }
    QMap<QString, QString> hash2FirstDuplicateFile;
    QSet<QString> firstDuplicateFileMd5;
    for (const auto& absPath : it.value()) {
      const QString& md5 = MD5Calculator::GetFileMD5(absPath);
      auto sameMd5It = hash2FirstDuplicateFile.find(md5);
      if (sameMd5It != hash2FirstDuplicateFile.cend()) {
        if (firstDuplicateFileMd5.find(md5) == firstDuplicateFileMd5.end()) {
          dupImgs.append(REDUNDANT_IMG_INFO{sameMd5It.value(), QFile{sameMd5It.value()}.size(), md5});
          firstDuplicateFileMd5.insert(md5);
        }
        dupImgs.append(REDUNDANT_IMG_INFO{absPath, QFile{absPath}.size(), md5});
      } else {
        hash2FirstDuplicateFile[md5] = absPath;
      }
    }
  }
  return dupImgs;
}
