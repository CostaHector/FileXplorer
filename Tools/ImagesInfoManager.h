#ifndef IMAGESINFOMANAGER_H
#define IMAGESINFOMANAGER_H
#include <QSet>
#include <QString>
#include "SingletonManager.h"

struct REDUNDANT_IMG_INFO {
  QString filePath;
  qint64 size;
  QString md5;
};

using RedundantImagesList = QList<REDUNDANT_IMG_INFO>;

struct IMG_INFO_DATA_T {
  int countHash() const { return m_commonFileHash.size(); }
  int countSize() const { return m_commonFileSizeSet.size(); }
  int size() const { return countHash() + countSize(); }
  void swap(IMG_INFO_DATA_T& rhs) {
    m_commonFileHash.swap(rhs.m_commonFileHash);
    m_commonFileSizeSet.swap(rhs.m_commonFileSizeSet);
  }

  bool contains(const QString& hash) const { return m_commonFileHash.contains(hash); }
  void insert(const QString& hash) { m_commonFileHash.insert(hash); }
  QSet<QString> m_commonFileHash;

  bool contains(const qint64& sz) const { return m_commonFileSizeSet.contains(sz); }
  void insert(const qint64& sz) { m_commonFileSizeSet.insert(sz); }
  QSet<qint64> m_commonFileSizeSet;
};

class ImagesInfoManager;
extern template class SingletonManager<ImagesInfoManager, IMG_INFO_DATA_T>;

class ImagesInfoManager final : public SingletonManager<ImagesInfoManager, IMG_INFO_DATA_T> {
 public:
  friend class SingletonManager<ImagesInfoManager, IMG_INFO_DATA_T>;
  IMG_INFO_DATA_T& ImgDataStruct() { return data(); }
  const IMG_INFO_DATA_T& ImgDataStruct() const { return data(); }

  IMG_INFO_DATA_T ReadOutImgsInfo() const;
  int ForceReloadImpl();

  // only used in decide by Benchmark library
  RedundantImagesList FindRedunImgs(const QString& folderPath, const bool bAlsoFindEmpty = true) const;
  QString GetDynRedunPath() const;

 private:
  ImagesInfoManager();
  ImagesInfoManager(const ImagesInfoManager& rhs) noexcept = delete;
  void InitializeImpl(const QString& path);
#ifdef RUNNING_UNIT_TESTS
  int ResetStateForTestImpl(const QString& localFilePath);
#endif
};

// only used in decide by MD5
RedundantImagesList FindDuplicateImgs(const QString& folderPath, const bool bAlsoFindEmpty = true);
#endif  // IMAGESINFOMANAGER_H
