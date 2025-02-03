#ifndef HARFILES_H
#define HARFILES_H

#include <QString>
#include <QMap>
#include <QSet>
#include <QFileInfo>

struct HAR_FILE_ITEM {
  QString name;
  QByteArray content;
  QString type;
  QString url;
};

const QStringList HAR_VERTICAL_HEAD {"Name", "Size(Byte)", "Type", "Url"};
const QSet<QString> IMAGE_PREVIEW_SUPPORTED {".jpeg", ".jpg", ".png", ".webp"};

class HarFiles
{
 public:
  HarFiles();
  void init();
  int size() const {return mHarItems.size();}
  bool empty() const {return mHarItems.isEmpty();}
  void swap(HarFiles& rhs);
  const HAR_FILE_ITEM& operator[](int i) const;
  bool operator()(const QString& harAbsPath);
  int SaveToLocal(QString dstRootpath = "", const QList<int>& selectedRows = {});
  static bool IsHarFile(const QFileInfo& fi);
 private:
  QString mHarFilePath;
  QList<HAR_FILE_ITEM> mHarItems;
  static const QMap<QString, QString> SUPPORTED_MIMETYPES;
};

#endif // HARFILES_H
