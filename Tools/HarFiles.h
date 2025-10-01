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

QString GetPathStem(const QString& url);
class HarFiles {
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
  void clear() {mHarItems.clear();}
private:
  QString mHarFilePath;
  QList<HAR_FILE_ITEM> mHarItems;
  static const QMap<QString, QString> SUPPORTED_MIMETYPES;
};

#endif // HARFILES_H
