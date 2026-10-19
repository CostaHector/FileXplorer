#ifndef HARFILES_H
#define HARFILES_H

#include <QString>
#include <QMap>

struct HAR_FILE_ITEM {
  QString name;
  QByteArray content;
  QString type;
  QString url;
};

class QFileInfo;

QString GetPathItemName(const QString& url);
class HarFiles {
public:
  void init();
  int size() const {return mHarItems.size();}
  bool empty() const {return mHarItems.isEmpty();}
  void swap(HarFiles& rhs);
  const HAR_FILE_ITEM& operator[](int i) const;
  void setHarAbsPath(const QString& harAbsPath) { mHarFilePath = harAbsPath; }
  bool operator()(const QString& harAbsPath);
  QString harFileFullPath() const { return mHarFilePath; }
  QString baseFileFullPath(int r) const;

  int SaveToLocal(QString dstRootpath = "", const QList<int>& selectedRows = {});
  static bool IsHarFile(const QFileInfo& fi);
  void clear() { mHarItems.clear(); }
private:
  bool ParseFilesInJsonByteArray(const QByteArray& jsonByteArray);

  QString mHarFilePath;
  QList<HAR_FILE_ITEM> mHarItems;
  static const QMap<QString, QString> SUPPORTED_MIMETYPES;
};

#endif // HARFILES_H
