#ifndef JSONTABLEMODEL_H
#define JSONTABLEMODEL_H
#include "QAbstractTableModelPub.h"
#include "JsonPr.h"
#include <QFileInfo>
#include <bitset>

class JsonTableModel : public QAbstractTableModelPub {
 public:
  using QAbstractTableModelPub::QAbstractTableModelPub;
  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return mCachedJsons.size(); }
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return JsonKey::JSON_TABLE_HEADERS_COUNT; }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  const QString& rootPath() const {return mRootPath;}
  int setRootPath(const QString& path, bool isForce = false);
  int forceReloadPath();

  QFileInfo fileInfo(const QModelIndex& index) const;
  QString filePath(const QModelIndex& index) const;
  QString fileName(const QModelIndex& index) const;
  QString fileBaseName(const QModelIndex& index) const;
  QString absolutePath(const QModelIndex& index) const;
  QString fullInfo(const QModelIndex& index) const;
  QStringList rel2fileNames(const QModelIndexList& indexes) const;

  int SetStudio(const QModelIndexList& rowIndexes, const QString& studio);
  int SetCastOrTags(const QModelIndexList& rowIndexes, const JSON_KEY_E keyEnum, const QString& sentence);
  int AddCastOrTags(const QModelIndexList& rowIndexes, const JSON_KEY_E keyEnum, const QString& sentence);
  int RmvCastOrTags(const QModelIndexList& rowIndexes, const JSON_KEY_E keyEnum, const QString& oneElement);
  int InitCastAndStudio(const QModelIndexList& rowIndexes);
  int HintCastAndStudio(const QModelIndexList& rowIndexes, const QString& sentence);
  int FormatCast(const QModelIndexList& rowIndexes);
  int UpdateFizeSizeField(const QModelIndexList& rowIndexes, const int ITERATE_FOLDER_FIRST_LIMIT=50);
  int UpdateDurationField(const QModelIndexList& rowIndexes, const int ITERATE_FOLDER_FIRST_LIMIT=50);
  int UpdateMD5Field(const QModelIndexList& rowIndexes, const int ITERATE_FOLDER_FIRST_LIMIT=50);
  int SyncFieldNameByJsonBaseName(const QModelIndexList& rowIndexes);
  int AppendCastFromSentence(const QModelIndex& ind, const QString& sentence, bool isUpperCaseSentence);
  int SetRecordContentsFixed(const QModelIndexList& rowIndexes, bool bFixed=true);

  int AfterJsonFilesNameRenamed(const QModelIndexList& indexes);
  int SaveCurrentChanges(const QModelIndexList& rowIndexes);
  std::pair<int, int> ExportCastStudioToLocalDictionaryFile(const QModelIndexList& rowIndexes) const;

  JsonPr GetJsonPr(const QModelIndex& ind) const;

  Qt::ItemFlags flags(const QModelIndex& /*index*/) const override {
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEditable;
  }

 private:
  int JsonFieldValueUpdateCore(const QModelIndexList& rowIndexes, JSON_KEY_E field, const int ITERATE_FOLDER_FIRST_LIMIT=50);
  QHash<QString, QString> GetVidBaseName2FullPath() const;
  bool setModified(int row, bool modified = true);
  bool setModifiedNoEmit(int row, bool modified = true);
  QVector<JsonPr> mCachedJsons;
  std::bitset<1000> m_modifiedRows;
  QString mRootPath;
};

#endif  // JSONTABLEMODEL_H
