#ifndef HARPARSERHELPER_H
#define HARPARSERHELPER_H

#include <QString>
#include <QMap>

struct HAR_FILE_ITEM {
  QString name;
  QByteArray content;
  QString type;
  QString url;
};

const QStringList HAR_VERTICAL_HEAD {"Name", "Size(Byte)", "Type", "Url"};

class HarParserHelper
{
 public:
  HarParserHelper();
  void init();
  int size() const {return mHarItems.size();}
  bool empty() const {return mHarItems.isEmpty();}
  void swap(HarParserHelper& rhs);
  const HAR_FILE_ITEM& operator[](int i) const;
  bool operator()(const QString& harAbsPath);
  int SaveToLocal(QString dstRootpath = "", const QList<int>& selectedRows = {});
 private:
  QString mHarFilePath;
  QList<HAR_FILE_ITEM> mHarItems;
  static const QMap<QString, QString> SUPPORTED_MIMETYPES;
};

#endif // HARPARSERHELPER_H
