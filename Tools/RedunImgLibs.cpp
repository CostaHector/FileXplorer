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
  QDirIterator it(folderPath, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    QFileInfo imgFi(it.filePath());
    const QString fileAbsPath = imgFi.absoluteFilePath();
    const qint64 sz = imgFi.size();
    m_commonFileSizeSet.insert(sz);
    const QString& md5 = MD5Calculator::GetFileMD5(fileAbsPath);
    m_commonFileHash.insert(md5);
  }
  qDebug("redundant image info size[%d] and hash[%d]", m_commonFileSizeSet.size(), m_commonFileHash.size());
  return m_commonFileHash.size();
}

REDUNDANT_IMG_BUNCH RedunImgLibs::FindRedunImgs(const QString& folderPath) {
  QDirIterator it(folderPath, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  REDUNDANT_IMG_BUNCH redundantImgs;
  const bool recycleEmptyImage = PreferenceSettings().value(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.name, RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.v).toBool();
  while (it.hasNext()) {
    it.next();
    QFileInfo imgFi(it.filePath());
    const QString fileAbsPath = imgFi.absoluteFilePath();
    const qint64 sz = imgFi.size();
    if (sz == 0) {
      if (recycleEmptyImage) {
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
