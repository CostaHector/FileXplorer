#ifndef ARCHIVEFILES_H
#define ARCHIVEFILES_H

#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QString>
#include <QVariant>
#include "Logger.h"

class ArchiveImagesRecusive;

class FilesListBase {
 public:
  FilesListBase() = default;
  virtual ~FilesListBase() = default;
  virtual bool isEmpty() const = 0;
  virtual int size() const = 0;
  virtual QVariant operator[](const int i) const = 0;
  virtual void clear() = 0;
};

class PlainStringList : public FilesListBase {
 public:
  PlainStringList(const QStringList& lst) : FilesListBase(), m_lst{lst} {}
  PlainStringList(QStringList&& lst) : FilesListBase(), m_lst{lst} {}

  bool isEmpty() const override { return m_lst.isEmpty(); }
  int size() const override { return m_lst.size(); }
  QVariant operator[](const int i) const override { return m_lst[i]; }
  void clear() override { m_lst.clear(); }

  QStringList m_lst;
};

class ArchiveFilesReader : public FilesListBase {
 public:
  static bool isQZFile(const QString& path);
  static bool isQZFile(const QFileInfo& fi);
  ArchiveFilesReader() = default;
  ~ArchiveFilesReader() = default;
  bool ReadAchiveFile(const QString& qzFilePath);
  bool DecompressToPath(const QString& dstPath) const;

  bool isEmpty() const override { return m_items.isEmpty(); }
  int size() const override { return m_items.size(); }
  QVariant operator[](const int i) const override { return m_items[i].m_datas; }
  void clear() override {
    m_fi.setFileName("");
    m_fi.close();
    m_ds.setDevice(nullptr);
    m_items.clear();
  }

  const QString& key(int i) const { return m_items[i].m_names; }
  qint64 beforeSize(int i) const { return m_items[i].m_beforeSize; }
  int afterSize(int i) const { return m_items[i].m_afterSize; }
  const QByteArray& value(int i) const { return m_items[i].m_datas; }

 private:
  bool ReadAnItem();

  QFile m_fi;
  QDataStream m_ds{nullptr};

  struct CompressedItem {
    QString m_names;
    qint64 m_beforeSize;
    int m_afterSize;
    QByteArray m_datas;
  };
  QList<CompressedItem> m_items;
#ifdef RUNNING_UNIT_TESTS
  void mockCompressedItem(QList<CompressedItem> newCompressedItems) {
    m_items = newCompressedItems;
  }
#endif
  static constexpr int INFO_ITEMS_CNT = 4;
};

class ArchiveFilesWriter {
 public:
  ArchiveFilesWriter() = default;
  ~ArchiveFilesWriter() = default;

  int CompressNow(const QString& workPath, const QStringList& fileNames, const QString& destArchiveFileName, bool enableAppend = true);

 private:
  inline void WriteIntoFile(const QString& name, const qint64& beforeSize, const QByteArray& fileData) {
    m_ds << name << beforeSize << fileData.size() << fileData;
  }
  static bool IsNeedCompress(const QFile& fi);

  QFile m_fi;
  QDataStream m_ds{nullptr};

  int AddOneFolder(const QString& workPath, const QString& rel2FolderName);
  int AddOneFile(const QString& workPath, const QString& rel2FileName);
};

class ArchiveImagesRecusive {
 public:
  ArchiveImagesRecusive(bool autoRecycle = true) : m_autoRecycle{autoRecycle} {}
  int CompressImgRecur(const QString& rootPath);
  bool CompressSubfolder(const QString& path, const QString& qzBaseName);

 private:
  QStringList m_allPres, m_allNames;
  const bool m_autoRecycle;
  static void setMaxCompressImageCnt(int newImgLimitCnt) {MAX_COMPRESSED_IMG_CNT = newImgLimitCnt;}
  static int MAX_COMPRESSED_IMG_CNT;
};

#endif  // ARCHIVEFILES_H
