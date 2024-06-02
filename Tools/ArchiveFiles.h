#ifndef ARCHIVEFILES_H
#define ARCHIVEFILES_H

#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QString>

class ArchiveFiles {
 public:
  enum OPERATION_TYPE { A_FOLDER = 0, FILES };
  enum COMPRESS_FILETYPE_FILTER { NO_FILTER = 0, ONLY_IMAGE, ONLY_PLAIN_TEXT };

  static int CompressImagesByGroup(const QString& rootPath, bool recycleAfterCompressed = true);

  ArchiveFiles(const QString& achieveName, const COMPRESS_FILETYPE_FILTER& compressFileType = NO_FILTER);
  ~ArchiveFiles();

  QVariantList PreviewFirstKItems(int k = -1);
  bool ReadFirstKItemsOut(int k, QStringList& paths, QByteArrayList& datas);

  bool CompressNow(OPERATION_TYPE type, const QStringList& paths, bool enableAppend = true);
  bool DecompressToPath(const QString& dstPath);

  bool deleteAchieveFile() const {
    if (not m_fi.exists()) {
      qDebug("achieve file[%s] already not exist", qPrintable(m_fi.fileName()));
      return false;
    }
    return QFile::moveToTrash(m_fi.fileName());
  }

  QByteArray compress(const QByteArray& source) const {
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

 private:
  static bool CompressADirectlyPath(const QString& path, const QString& qzBaseName, QString& allPres, QString& allNames);

  bool IsNeedCompress(const QString& suffix) const;
  QStringList GetCompressType() const;
  bool AppendAFolder(const QStringList& paths);
  bool AppendFiles(const QStringList& filesPath);

  QFile m_fi;
  QDataStream m_ds;
  const COMPRESS_FILETYPE_FILTER m_compressFilesType;

  static constexpr int PLAIN_TEXT_FILE_COMPRESS_LEVEL = -1;
};
#endif  // ARCHIVEFILES_H
