#ifndef REDUNIMGLIBS_H
#define REDUNIMGLIBS_H
#include <QSet>
#include <QString>

struct REDUNDANT_IMG_INFO {
  QString filePath;
  qint64 size;
  QString md5;
};

typedef QList<REDUNDANT_IMG_INFO> REDUNDANT_IMG_BUNCH;

struct RedunImgLibs {
  static REDUNDANT_IMG_BUNCH FindDuplicateImgs(const QString& folderPath, const bool bAlsoFindEmpty = true);

  RedunImgLibs(const QString& libPath = "") : m_libPath{libPath} {}
  static QString GetRedunPath();
  int ReadLib();
  int WriteLib();
  int LearnSizeAndHashFromRedunImgPath(const QString& folderPath);
  REDUNDANT_IMG_BUNCH FindRedunImgs(const QString& folderPath, const bool bAlsoFindEmpty = true);
  const QString m_libPath;
  QSet<qint64> m_commonFileSizeSet;
  QSet<QString> m_commonFileHash;
};
#endif  // REDUNIMGLIBS_H
