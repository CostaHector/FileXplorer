#ifndef ARCHIVEFILES_H
#define ARCHIVEFILES_H

#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QString>
#include <QVariant>
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
  virtual bool isEmpty() const override { return m_lst.isEmpty(); }
  virtual int size() const override { return m_lst.size(); }
  virtual QVariant operator[](const int i) const override { return m_lst[i]; }
  virtual void clear() override { m_lst.clear(); }

  QStringList m_lst;
};

class ArchiveFiles : public FilesListBase {
 public:
  friend class ArchiveImagesRecusive;
  enum OPERATION_TYPE { A_FOLDER = 0, FILES };
  enum COMPRESS_FILETYPE_FILTER { NO_FILTER = 0, ONLY_IMAGE, ONLY_PLAIN_TEXT };

  ArchiveFiles(const QString& achieveName = "", const COMPRESS_FILETYPE_FILTER& compressFileType = NO_FILTER);
  void swap(ArchiveFiles& rhs);

  ~ArchiveFiles();

  static bool isQZFile(const QFileInfo& fi);
  static bool isQZFile(const QString& path);

  bool CompressNow(OPERATION_TYPE type, const QStringList& paths, bool enableAppend = true);
  bool DecompressToPath(const QString& dstPath);

  bool deleteAchieveFile() {
    if (not m_fi.exists()) {
      qDebug("achieve file[%s] already not exist", qPrintable(m_fi.fileName()));
      return false;
    }
    return QFile::moveToTrash(m_fi.fileName());
  }

  QByteArray compress(const QByteArray& source) {
    switch (m_compressFilesType) {
      case ONLY_PLAIN_TEXT:
        return qCompress(source, PLAIN_TEXT_FILE_COMPRESS_LEVEL);
      default:
        return source;
    }
  }

  QByteArray decompress(const QByteArray& compressed) const {
    switch (m_compressFilesType) {
      case ONLY_PLAIN_TEXT:
        return qUncompress(compressed);
      default:
        return compressed;
    }
  }
  virtual void clear() override {
    m_fi.setFileName("");
    m_ds.setDevice(nullptr);
    m_names.clear();
    m_datas.clear();
  }
  virtual bool isEmpty() const override { return m_names.isEmpty(); }
  virtual int size() const override { return m_names.size(); }
  virtual QVariant operator[](const int i) const override { return m_datas[i]; }

  const QString& key(int i) const { return m_names[i]; }
  const qint64& beforeSize(int i) const { return m_beforeSize[i]; }
  const int& afterSize(int i) const { return m_afterSize[i]; }

  QByteArray value(int i) const { return m_datas[i].toByteArray(); }
  const QVariantList& getByteArrayList() const { return m_datas; }

 private:
  inline void WriteIntoFile(const QString& name, const qint64& beforeSize, const QByteArray& fileData) {
    m_ds << name << beforeSize << fileData.size() << fileData;
  }
  inline void Read() {
    QString relFilePath;
    qint64 beforeSize = 0;
    int afterSize = 0;
    QByteArray fileData;
    if (INFO_ITEMS_CNT == 4) {
      m_ds >> relFilePath >> beforeSize >> afterSize >> fileData;
    } else if (INFO_ITEMS_CNT == 2) {
      m_ds >> relFilePath >> fileData;
    }
    m_names.append(relFilePath);
    m_beforeSize.append(beforeSize);
    m_afterSize.append(afterSize);
    m_datas.append(decompress(fileData));
  }

  bool ReadItemsCount();

  bool IsNeedCompress(const QString& suffix) const;
  QStringList GetCompressType() const;
  bool AppendAFolder(const QStringList& paths);
  bool AppendFiles(const QStringList& filesPath);

  QFile m_fi;
  QDataStream m_ds;
  QStringList m_names;
  QList<qint64> m_beforeSize;
  QList<int> m_afterSize;
  QVariantList m_datas;

  COMPRESS_FILETYPE_FILTER m_compressFilesType;
  static constexpr int PLAIN_TEXT_FILE_COMPRESS_LEVEL = -1;
  static constexpr int MAX_COMPRESSED_IMG_CNT = 30;
  static constexpr int INFO_ITEMS_CNT = 4;
};

class ArchiveImagesRecusive {
 public:
  ArchiveImagesRecusive(bool autoRecycle = true) : m_autoRecycle{autoRecycle} {}
  int CompressImgRecur(const QString& rootPath);
  bool CompressSubfolder(const QString& path, const QString& qzBaseName);

 private:
  QStringList m_allPres, m_allNames;
  bool m_autoRecycle;
};

#endif  // ARCHIVEFILES_H
