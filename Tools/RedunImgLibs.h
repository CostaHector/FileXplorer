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

class RedunImgLibs {
public:
  static REDUNDANT_IMG_BUNCH FindDuplicateImgs(const QString& folderPath, const bool bAlsoFindEmpty = true);
  static QString GetRedunPath();
  static RedunImgLibs& GetInst(const QString& benchMarkPath = "");

  REDUNDANT_IMG_BUNCH FindRedunImgs(const QString& folderPath, const bool bAlsoFindEmpty = true);
private:
  RedunImgLibs() = default;
  int LearnSizeAndHashFromRedunImgPath(const QString& folderPath);

  QSet<qint64> m_commonFileSizeSet;
  QSet<QString> m_commonFileHash;
  static bool mBInited;
};
#endif  // REDUNIMGLIBS_H
