#ifndef ARCHIEVEFILES_H
#define ARCHIEVEFILES_H

#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QString>

class ArchieveFiles {
 public:
  ArchieveFiles(const QString& achieveName);

  bool appendAFolder(const QString& folderPath);
  bool appendAFile(const QString& filePath);
  bool appendFiles(const QStringList& filesPath);

  bool decompressToPath(const QString& dstPath);

  bool deleteAchieveFile() const {
    if (not QFileInfo(m_achievePath).isFile()) {
      qDebug("achieve file[%s] already not exist", qPrintable(m_achievePath));
      return false;
    }
    return QFile::moveToTrash(m_achievePath);
  }

  QByteArray compress(const QByteArray& source) const { return qCompress(source, PLAIN_TEXT_FILE_COMPRESS_LEVEL); }

  QByteArray decompress(const QByteArray& source) const { return qUncompress(source); }

 private:
  const QString m_achievePath;
  static constexpr int PLAIN_TEXT_FILE_COMPRESS_LEVEL = -1;
  QFile m_fi;
  QDataStream m_ds;
};

#endif  // ARCHIEVEFILES_H
